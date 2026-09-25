# frozen_string_literal: true

# PCRE2's Asciidoctor extension defines a small semantic inline vocabulary and
# backend-specific presentation for manpage output. Keep the extension
# backend-neutral until conversion so all outputs consume the same AST.
require 'asciidoctor/converter/manpage'
require 'asciidoctor/extensions'
# Decode Asciidoctor's special-character substitutions before Rouge lexing.
require 'cgi'
# Supply lexical tokens for terminal highlighting of C source blocks.
require 'rouge'

# Base processor for short API macros such as `func:pcre2_match()`. Each macro
# becomes a monospaced quoted node with a semantic role; converters decide how
# that role is presented instead of the macro injecting backend-specific text.
class Pcre2ApiInlineMacro < Asciidoctor::Extensions::InlineMacroProcessor
  use_dsl
  using_format :short

  def process(parent, target, _attributes)
    create_inline parent, :quoted, target,
      type: :monospaced, attributes: {'role' => @role}
  end
end

# Recognize public PCRE2 function names, including the source-visible `()`.
class Pcre2FunctionInlineMacro < Pcre2ApiInlineMacro
  named :func
  match /func:(pcre2_[a-z0-9_]+\(\))/

  def initialize(*args)
    super
    @role = 'function'
  end
end

# Recognize public PCRE2 types and POSIX types with a conventional `_t` suffix.
class Pcre2TypeInlineMacro < Pcre2ApiInlineMacro
  named :type
  match /type:(PCRE2_[A-Z0-9_]+|pcre2_[a-z0-9_]+|[A-Za-z][A-Za-z0-9_]*_t)/

  def initialize(*args)
    super
    @role = 'type'
  end
end

# Recognize uppercase public constants, options, flags, and error names.
class Pcre2ConstantInlineMacro < Pcre2ApiInlineMacro
  named :const
  match /const:([A-Z][A-Z0-9_]*)/

  def initialize(*args)
    super
    @role = 'constant'
  end
end

# Recognize lowercase function arguments, callback names, and member names.
class Pcre2ArgumentInlineMacro < Pcre2ApiInlineMacro
  named :arg
  match /arg:([a-z][a-z0-9_]*)/

  def initialize(*args)
    super
    @role = 'parameter'
  end
end

# Recognize character constants
class Pcre2CharacterInlineMacro < Pcre2ApiInlineMacro
  named :char
  match /char:((?:0x|U\+)[0-9A-Fa-f]+)/

  def initialize(*args)
    super
    @role = 'character'
  end
end

# Install the API macros globally for documents loaded with this extension.
Asciidoctor::Extensions.register do
  inline_macro Pcre2FunctionInlineMacro
  inline_macro Pcre2TypeInlineMacro
  inline_macro Pcre2ConstantInlineMacro
  inline_macro Pcre2ArgumentInlineMacro
  inline_macro Pcre2CharacterInlineMacro
end

# Preserve PCRE2's established terminal layout while adding description-list,
# inline API, and C source presentation to Asciidoctor's manpage backend.
# `ESC_BS`, inherited from the stock converter, protects intended roff escapes
# until `manify` performs its final escape and fake-markup cleanup pass.
class Pcre2ManpageConverter < Asciidoctor::Converter::ManPageConverter
  MIN_DLIST_WIDTH = 7
  MAX_DLIST_WIDTH = 26
  PROTECTED_FONT_MARKER_RX = %r(</?#{Regexp.escape ESC_BS}f(B|I|\(CR|P)>)

  # Replace the stock manpage converter for documents using this extension.
  register_for 'manpage'

  # Stock section conversion protects inline markup while uppercasing level-one
  # headings. Additionally preserve lowercase PCRE2 names in those headings.
  def uppercase_pcdata(text)
    protected = []
    protect = lambda do |fragment|
      protected << fragment
      "\0#{protected.length - 1}\0"
    end
    masked = text.gsub(/\bpcre2_[a-z0-9_]+(?:\(\))?|\bpcre2test\b/, &protect)
    super(masked).gsub(/\0(\d+)\0/) { protected[$1.to_i] }
  end

  def convert_dlist(node)
    return super if node.style == 'qanda'

    # PCRE2 uses traditional tagged paragraphs. `width` overrides the automatic
    # label width; fake inline-font tags are excluded from the measurement.
    result = []
    terms_and_descriptions = node.items.map do |terms, description|
      term = terms.map(&:text).join(', ')
      [term, description]
    end
    width = if node.attr?('width')
      node.attr('width').to_i
    else
      lengths = terms_and_descriptions.map {|term, _| term.gsub(/<[^>]+>/, '').length }
      fitting_lengths = lengths.select {|length| length + 1 <= MAX_DLIST_WIDTH }
      if fitting_lengths.length > lengths.length / 2
        [fitting_lengths.max + 1, MIN_DLIST_WIDTH].max
      else
        MIN_DLIST_WIDTH
      end
    end
    spaced = terms_and_descriptions.any? do |_, description|
      description && description.blocks?
    end
    # `.sp` retains normal separation from preceding prose, `.RS 4` indents the
    # whole list by four ens, and `.PD 0` suppresses inter-item paragraph
    # distance for lists whose descriptions contain no additional blocks.
    result << '.sp'
    result << '.RS 4'
    result << (spaced ? '.PD' : '.PD 0')
    terms_and_descriptions.each do |term, description|
      # `.TP width` emits a term followed by its indented description. The macro
      # resets adjustment, so restore ragged-right mode for every description.
      result << %(.TP #{width}
#{manify term, whitespace: :normalize}
.ad l)
      if description
        result << (manify description.text, whitespace: :normalize) if description.text?
        if description.blocks?
          # Block macros discard `.TP`'s transient description indent.
          result << ".RS #{width}"
          result << description.content
          result << '.RE'
        end
      end
    end
    result << '.PD'
    result << '.RE'
    # Close the list as a paragraph and restore ragged-right mode because `.PP`
    # otherwise switches subsequent prose back to full justification.
    result << %(.PP
.ad l)
    result.join "\n"
  end

  def convert_ulist(node)
    result = []
    result << %(.sp
.B #{manify node.title}
.br) if node.title?
    spaced = node.items.any?(&:blocks?)
    node.items.each_with_index do |item, index|
      # Keep compact lists together, but space every item if any item has
      # attached blocks so the list uses consistent vertical rhythm.
      result << '.sp' if index.zero? || spaced
      result << %[.RS 4
.ie n \\{\\
\\h'-04'\\(bu\\h'+03'\\c
.\\}
.el \\{\\
.  sp -1
.  IP \\(bu 2.3
.\\}#{(list_text = manify item.text, whitespace: :normalize).empty? ? '' : "\n#{list_text}"}]
      if item.blocks?
        item_content = item.content
        item_content = item_content.slice 4, item_content.length if list_text.empty? && (item_content.start_with? %(.sp\n))
        result << item_content
      end
      result << '.RE'
    end
    result.join "\n"
  end

  def convert_olist(node)
    result = []
    result << %(.sp
.B #{manify node.title}
.br) if node.title?
    spaced = node.items.any?(&:blocks?)
    start = (node.attr 'start', 1).to_i
    node.items.each_with_index do |item, index|
      result << '.sp' if index.zero? || spaced
      numeral = ordered_list_numeral(index + start, node.style)
      result << %(.RS 4
.ie n \\{\\
\\h'-04' #{numeral}.\\h'+01'\\c
.\\}
.el \\{\\
.  sp -1
.  IP " #{numeral}." 4.2
.\\}#{(list_text = manify item.text, whitespace: :normalize).empty? ? '' : "\n#{list_text}"})
      if item.blocks?
        item_content = item.content
        item_content = item_content.slice 4, item_content.length if list_text.empty? && (item_content.start_with? %(.sp\n))
        result << item_content
      end
      result << '.RE'
    end
    result.join "\n"
  end

  def ordered_list_numeral(number, style)
    return number.to_s unless number.positive?

    case style
    when 'loweralpha'
      alpha_list_numeral(number).downcase
    when 'upperalpha'
      alpha_list_numeral(number)
    when 'lowerroman'
      Asciidoctor::Helpers.int_to_roman(number).downcase
    when 'upperroman'
      Asciidoctor::Helpers.int_to_roman(number)
    else
      number.to_s
    end
  end

  def alpha_list_numeral(number)
    result = +''
    while number > 0
      number -= 1
      result.prepend((65 + (number % 26)).chr)
      number /= 26
    end
    result
  end

  # Flatten nested font changes into independent runs because roff's `\fP`
  # remembers only one previous font; it does not maintain a font stack.
  def convert_inline_quoted(node)
    case node.role
    when 'type', 'constant'
      # Gross hack to add line-break points for roff when inside a table cell
      text = node.parent.context == :table_cell ?
        node.text.gsub('_', "_#{ESC_BS}:") : node.text
      inline_font_run text, 'B'
    when 'parameter'
      inline_font_run node.text, 'I'
    when 'function'
      node.text
    else
      case node.type
      when :emphasis
        inline_font_run node.text, 'I'
      when :strong
        inline_font_run node.text, 'B'
      when :monospaced
        inline_font_run node.text, '(CR'
      when :single
        %[<#{ESC_BS}(oq>#{node.text}</#{ESC_BS}(cq>]
      when :double
        %[<#{ESC_BS}(lq>#{node.text}</#{ESC_BS}(rq>]
      else
        node.text
      end
    end
  end

  def inline_font_run(text, font)
    text.empty? ? '' : %(<#{ESC_BS}f#{font}>#{text}</#{ESC_BS}fP>)
  end

  # Asciidoctor finishes nested inline substitutions after the outer converter
  # callback. Flatten the resulting protected font markers before stock manify
  # turns them into roff escapes.
  def manify(text, options = {})
    super flatten_inline_fonts(text), options
  end

  def flatten_inline_fonts(text)
    return text unless PROTECTED_FONT_MARKER_RX.match? text

    result = []
    fonts = []
    offset = 0
    text.to_enum(:scan, PROTECTED_FONT_MARKER_RX).each do
      match = Regexp.last_match
      segment = text[offset...match.begin(0)]
      result << (fonts.empty? ? segment : inline_font_run(segment, fonts.last))
      if match[1] == 'P'
        raise 'Unbalanced protected roff font markers' if fonts.empty?

        fonts.pop
      else
        fonts << match[1]
      end
      offset = match.end(0)
    end
    segment = text[offset..]
    result << (fonts.empty? ? segment : inline_font_run(segment, fonts.last))
    raise 'Unbalanced protected roff font markers' unless fonts.empty?

    result.join
  end

  def convert_literal(node)
    convert_preformatted node, manify(node.content, whitespace: :preserve)
  end

  def convert_listing(node)
    content = node.style == 'source' ?
      convert_source(node) : manify(node.content, whitespace: :preserve)
    convert_preformatted node, content
  end

  # Follow stock literal/listing conversion, but keep preformatted blocks at
  # the surrounding margin instead of adding a four-en relative indent.
  def convert_preformatted(node, content)
    result = []
    if node.title?
      title = node.context == :listing ? node.captioned_title : node.title
      result << %(.sp
.B #{manify title}
.br)
    end
    # Restore sentence spacing so groff preserves aligned spaces after punctuation.
    formatted = %(.sp
.ss \\n[.ss]
.nf
.fam C
#{content}
.fam
.fi
.ss \\n[.ss] 0)
    if node.has_role?('wide')
      formatted = [
        '.nr pI \\n(.i',
        '.in 0',
        formatted,
        '.in \\n(pIu',
        '.rr pI',
      ].join "\n"
    end
    result << formatted
    result.join "\n"
  end

  # Highlight C source with Rouge before the single manify escaping pass. Other
  # languages keep normal source rendering. Asciidoctor callouts already contain
  # protected roff escapes, so those blocks deliberately use the unhighlighted
  # path to preserve callout markers and their list linkage.
  def convert_source(node)
    return manify(node.content, whitespace: :preserve) unless node.attr('language') == 'c'
    return manify(node.content, whitespace: :preserve) if node.content.include? ESC_BS

    tokens = Rouge::Lexers::C.lex(CGI.unescapeHTML(node.content)).to_a
    highlighted = tokens.each_with_index.map do |(token, text), index|
      font = c_token_font(token, text, tokens, index, node.has_role?('prototype'))
      font ? apply_roff_font(text, font) : text
    end.join
    manify highlighted, whitespace: :preserve
  end

  # Wrap only visible token text in Asciidoctor's protected fake font tags,
  # leaving surrounding whitespace outside to avoid font-only lines. A leading
  # preprocessor `#` needs a zero-width roff escape or manify mistakes it for
  # part of the fake markup and removes it.
  def apply_roff_font(text, font)
    match = text.match(/\A(\s*)(#?)(.*?)(\s*)\z/m)
    return text if match[3].empty?

    marker = match[2].empty? ? '' : %(#{ESC_BS}&#)
    %(#{match[1]}<#{ESC_BS}f#{font}>#{marker}#{match[3]}</#{ESC_BS}fP>#{match[4]})
  end

  # Map Rouge's C tokens onto the terminal's limited font vocabulary. Rouge
  # cannot infer project typedefs, macro constants, or call arguments from C,
  # so PCRE2/all-uppercase naming supplies types and constants. The `prototype`
  # flag comes from `[source,c,role=prototype]`; only those blocks treat the
  # remaining generic names as italic parameters.
  def c_token_font(token, text, tokens, index, prototype)
    token_name = token.qualname
    return 'B' if token_name.start_with?('Keyword') ||
      token_name.start_with?('Name.Constant') ||
      token_name.start_with?('Name.Builtin') ||
      token_name.start_with?('Comment.Preproc')
    return 'I' if token_name.start_with?('Name.Variable')
    return unless token_name == 'Name'
    return 'B' if text.match?(/\A[A-Z][A-Z0-9_]*\z/)
    return function_call?(tokens, index) ? nil : 'B' if text.start_with?('pcre2_')

    'I' if prototype
  end

  # A lowercase `pcre2_` name followed by `(` is a call and remains roman;
  # otherwise that naming form denotes a PCRE2 type and is bold.
  def function_call?(tokens, index)
    following = tokens.drop(index + 1).find {|token, text| token.qualname != 'Text' || !text.strip.empty? }
    following && following[1].start_with?('(')
  end
end