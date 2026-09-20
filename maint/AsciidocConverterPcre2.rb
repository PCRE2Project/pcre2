# frozen_string_literal: true

# PCRE2's Asciidoctor extension defines a small semantic inline vocabulary and
# backend-specific presentation for manpage and HTML output. Keep the extension
# backend-neutral until conversion so both outputs consume the same AST.
require 'asciidoctor/converter/manpage'
require 'asciidoctor/converter/html5'
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

# Recognize uppercase and lowercase forms used for public PCRE2 types.
class Pcre2TypeInlineMacro < Pcre2ApiInlineMacro
  named :type
  match /type:(PCRE2_[A-Z0-9_]+|pcre2_[a-z0-9_]+)/

  def initialize(*args)
    super
    @role = 'type'
  end
end

# Recognize uppercase public constants, options, flags, and error names.
class Pcre2ConstantInlineMacro < Pcre2ApiInlineMacro
  named :const
  match /const:(PCRE2_[A-Z0-9_]+)/

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

# Install the API macros globally for documents loaded with this extension.
Asciidoctor::Extensions.register do
  inline_macro Pcre2FunctionInlineMacro
  inline_macro Pcre2TypeInlineMacro
  inline_macro Pcre2ConstantInlineMacro
  inline_macro Pcre2ArgumentInlineMacro
end

# Preserve PCRE2's established terminal layout while adding semantic list,
# inline API, and C source presentation to Asciidoctor's manpage backend.
# `ESC_BS`, inherited from the stock converter, protects intended roff escapes
# until `manify` performs its final escape and fake-markup cleanup pass.
class Pcre2ManpageConverter < Asciidoctor::Converter::ManPageConverter
  # Dense API reference lists opt into compact, indented presentation; ordinary
  # prose description lists retain normal spacing and margins.
  SEMANTIC_DLIST_ROLES = %w[parameters members options values returns errors].freeze

  # Replace the stock manpage converter for documents using this extension.
  register_for 'manpage'

  def convert_section(node)
    # Stock manpage conversion uppercases level-one titles after AsciiDoc has
    # parsed them. Preserve inline code and backslash sequences while matching
    # the historical all-caps rendering of ordinary heading text.
    macro = node.level > 1 ? 'SS' : 'SH'
    title = node.level > 1 ? node.captioned_title : uppercase_title(node.title)
    %(.#{macro} "#{manify title}"
#{node.content})
  end

  # Uppercase ordinary level-one heading text while protecting inline markup,
  # regex property escapes, and lowercase PCRE2 names from case conversion.
  def uppercase_title(title)
    protected = []
    protect = lambda do |fragment|
      protected << fragment
      "\0#{protected.length - 1}\0"
    end
    masked = title.gsub(/<[^>]+>.*?<\/[^>]+>/, &protect)
    masked = masked.gsub(/\\[pP] and \\[pP]/, &protect)
    masked = masked.gsub('\\p', &protect).gsub('\\P', &protect)
    masked = masked.gsub(/\bpcre2_[a-z0-9_]+(?:\(\))?|\bpcre2test\b/, &protect)
    masked.upcase.gsub(/\0(\d+)\0/) { protected[$1.to_i] }
  end

  def convert_dlist(node)
    # PCRE2 uses traditional tagged paragraphs. `width` (or legacy `indent`)
    # overrides an automatic label width of longest visible term plus one.
    # Capping it at 24 keeps long API names from consuming the description area;
    # fake inline-font tags are excluded from the measurement.
    result = []
    terms_and_descriptions = node.items.map do |terms, description|
      term = terms.map(&:text).join(', ')
      [term, description]
    end
    width = if node.attr?('width') || node.attr?('indent')
      node.attr('width', node.attr('indent')).to_i
    else
      longest_term = terms_and_descriptions.map {|term, _| term.gsub(/<[^>]+>/, '').length }.max || 0
      [longest_term + 1, 24].min
    end
    semantic = !(node.roles & SEMANTIC_DLIST_ROLES).empty?
    # `.sp` retains normal separation from preceding prose, `.RS 4` indents the
    # whole list by four ens, and `.PD 0` suppresses only inter-item paragraph
    # distance. A bare `.PD` below restores the man macro package's default.
    result << '.sp' if semantic
    result << '.RS 4' if semantic
    result << '.PD 0' if semantic
    terms_and_descriptions.each do |term, description|
      # `.TP width` emits a term followed by its indented description. The macro
      # resets adjustment, so restore ragged-right mode for every description.
      result << %(.TP #{width}
#{manify term, whitespace: :normalize}
.ad l)
      if description
        result << (manify description.text, whitespace: :normalize) if description.text?
        result << description.content if description.blocks?
      end
    end
    result << '.PD' if semantic
    result << '.RE' if semantic
    # Close the list as a paragraph and restore ragged-right mode because `.PP`
    # otherwise switches subsequent prose back to full justification.
    result << %(.PP
.ad l)
    result.join "\n"
  end

  # Present semantic API roles without changing generic quoted-node behavior:
  # types/constants are bold, parameters italic, and functions plain roman.
  def convert_inline_quoted(node)
    case node.role
    when 'type', 'constant'
      %(<#{ESC_BS}fB>#{node.text}</#{ESC_BS}fP>)
    when 'parameter'
      %(<#{ESC_BS}fI>#{node.text}</#{ESC_BS}fP>)
    when 'function'
      node.text
    else
      super
    end
  end

  def convert_literal(node)
    # Stock literal blocks are indented monospaced listings. Synopsis blocks
    # need the PCRE2 troff layout below. All other PCRE2 literal blocks are
    # formatted prose; their substitutions are declared in the AsciiDoc source.
    return convert_synopsis(node) if node.style == 'synopsis'

    convert_formatted node
  end

  def convert_synopsis(node)
    # Unlike stock literal output, do not surround the synopsis with `.RS 4`.
    # Function declarations are intentionally aligned at the page margin.
    # Temporarily undo Asciidoctor's sentence-space suppression so punctuation
    # cannot normalize deliberately aligned spaces in no-fill content.
    %(.sp
.ss \\n[.ss]
.nf
.fam C
#{manify node.content, whitespace: :preserve}
.fam
.fi
.ss \\n[.ss] 0)
  end

  def convert_formatted(node)
    # Unlike stock literal output, render aligned reference rows in the normal
    # font with a two-space default indent. `.br` preserves each source row
    # without turning it into a monospaced code block; `indent` overrides it.
    content = node.content
    lines = content.lines(chomp: true)
    indent = node.attr('indent', 2)
    %(.sp
#{lines.map {|line| ".ti +#{indent}\n#{manify line, whitespace: :preserve}" }.join "\n.br\n"})
  end

  def convert_listing(node)
    # Stock listings are indented with `.if n .RS 4`. PCRE2 source examples,
    # including long function declarations, deliberately use the full page
    # width and preserve their source alignment; non-source listings retain
    # the stock implementation.
    return super unless node.style == 'source'

    result = []
    result << %(.sp
.B #{manify node.captioned_title}
.br) if node.title?
    # Asciidoctor globally disables additional sentence spacing, which makes
    # groff normalize runs of spaces after punctuation even in no-fill mode.
    # Restore normal spacing for exact source columns, then reinstate that
    # document-wide setting after the listing.
    source = %(.sp
.ss \\n[.ss]
.nf
.fam C
#{convert_source(node)}
.fam
.fi
.ss \\n[.ss] 0)
    # The fixed-width pcre2demo source must use the full width of an 80-column
    # terminal. Save the current indent, move to the page edge, then restore
    # the saved value after the block.
    source = [
      '.nr pI \\n(.i',
      '.in 0',
      source,
      '.in \\n(pIu',
      '.rr pI',
    ].join "\n" if node.has_role?('wide')
    result << source
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

# Preserve PCRE2's dedicated synopsis structure in HTML; all other HTML nodes,
# including ordinary literal blocks and semantic inline roles, use stock
# conversion.
class Pcre2Html5Converter < Asciidoctor::Converter::Html5Converter
  # Replace the stock HTML5 converter for documents using this extension.
  register_for 'html5'

  def convert_literal(node)
    # Preserve the dedicated synopsis markup. Stock conversion keeps spacing,
    # titles, IDs, and roles intact for all ordinary literal blocks.
    return convert_synopsis(node) if node.style == 'synopsis'

    super
  end

  def convert_synopsis(node)
    # Keep the conventional listingblock structure, but add PCRE2's synopsis
    # class and a `<code>` wrapper for stylesheet and semantic targeting.
    %(<div class="listingblock synopsis">
<div class="content">
<pre class="nowrap"><code>#{node.content}</code></pre>
</div>
</div>)
  end

end