#!/usr/bin/env ruby
# frozen_string_literal: true

# PCRE2's Asciidoctor extension defines a small semantic inline vocabulary and
# backend-specific presentation for HTML and manpage output. Keep the extension
# backend-neutral until conversion so all outputs consume the same AST.
require 'asciidoctor'
require 'asciidoctor/converter/manpage'
require 'asciidoctor/extensions'
require 'asciidoctor/syntax_highlighter/rouge'
require 'asciidoctor-html5s'
# Decode Asciidoctor's special-character substitutions before Rouge lexing.
require 'cgi'
# Supply lexical tokens for HTML and terminal source highlighting.
require 'rouge'
require 'optparse'
require 'fileutils'
require 'date'
require 'nokogiri'
require 'unicode/name'
require_relative 'UpdateCommon'

class Pcre2DocumentDefaults < Asciidoctor::Extensions::Preprocessor
  def process(document, reader)
    document.set_attribute 'lang', 'en' unless document.attr? 'lang'
    document.set_attribute 'source-highlighter', 'rouge' unless document.attr? 'source-highlighter'
    if document.basebackend? 'html'
      document.set_attribute 'toc-title', 'Contents'
      document.set_attribute 'nofooter' unless document.attr? 'nofooter'
      document.set_attribute 'stylesheet', 'css/document.css'
      document.set_attribute 'linkcss'
      document.delete_attribute 'copycss'
      document.set_attribute 'docrole', 'offline-manual'
    end
    reader
  end
end

class Pcre2HtmlPostprocessor < Asciidoctor::Extensions::Postprocessor
  def process(document, output)
    return output unless document.basebackend? 'html'

    Pcre2HtmlOutput.process(document, output)
  end
end

module Pcre2Source
  SEMANTIC_ROLES = %w[regex subject repl].freeze
  PLAIN_LANGUAGES = ['', 'text', 'plain', 'plaintext', *SEMANTIC_ROLES].freeze
  SEMANTIC_TOKEN = Rouge::Token['Generic'].make_token(:Pcre2Semantic, 'pcre2-semantic')
  KEYWORD_CONSTANTS = %w[TRUE FALSE NULL true false nullptr].freeze
  ROLE_TOKENS = {
    'function' => 'Name.Function',
    'type' => 'Keyword.Type',
    'constant' => 'Name.Constant',
    'parameter' => 'Name.Variable',
    'character' => 'Literal.String.Char',
  }.merge(SEMANTIC_ROLES.to_h { |role| [role, SEMANTIC_TOKEN.qualname] }).freeze
  PROFILE_FONTS = {
    default: '(CR',
    keyword: '(CR',
    literal: '(CR',
    preprocessor: '(CR',
    constant: '(CB',
    type: '(CR',
    function: '(CB',
    comment: '(CI',
    variable: '(CI',
    semantic: '(CI',
  }.freeze

  module_function

  def inline_token(roles, text)
    return SEMANTIC_TOKEN if (roles & SEMANTIC_ROLES).any?

    role = roles.find { |name| ROLE_TOKENS.key?(name) }
    return unless role

    name = role == 'constant' && KEYWORD_CONSTANTS.include?(text) ?
      'Keyword.Constant' : ROLE_TOKENS.fetch(role)
    Rouge::Token[name]
  end

  def lexer(language)
    return if PLAIN_LANGUAGES.include?(language.to_s)

    Rouge::Lexer.find_fancy(language) || raise(ArgumentError, "Unknown source language: #{language.inspect}")
  end

  def inline_language(roles)
    return if (roles & SEMANTIC_ROLES).any?

    roles.find { |role| !PLAIN_LANGUAGES.include?(role) && Rouge::Lexer.find(role) }
  end

  def tokens(source, language)
    return [[SEMANTIC_TOKEN, source]] if SEMANTIC_ROLES.include?(language.to_s)

    parser = lexer(language)
    return [[Rouge::Token['Text'], source]] unless parser

    result = parser.lex(source).to_a
    return result unless parser.tag == 'c'

    result = result.each_with_index.map do |(token, text), index|
      if Rouge::Token['Keyword'].matches?(token) && text == 'const'
        next [Rouge::Token['Keyword.Type'], text]
      end
      if Rouge::Token['Name'].matches?(token) && KEYWORD_CONSTANTS.include?(text)
        next [Rouge::Token['Keyword.Constant'], text]
      end
      next [token, text] unless token.qualname == 'Name'

      following = result.drop(index + 1).find do |next_token, value|
        !next_token.qualname.start_with?('Comment') && !value.strip.empty?
      end
      category = if following && following[1].start_with?('(')
        'Name.Function'
      elsif text.start_with?('pcre2_') || text.end_with?('_t') ||
          text.match?(/\APCRE2_(?:SIZE|(?:UCHAR|SPTR)(?:8|16|32)?)\z/)
        'Keyword.Type'
      elsif text.match?(/\A[A-Z][A-Z0-9_]*\z/)
        'Name.Constant'
      else
        'Name.Variable'
      end
      [Rouge::Token[category], text]
    end

    result.each_with_index.flat_map do |(token, text), index|
      next [[token, text]] unless Rouge::Token['Operator'].matches?(token)

      following = result[index + 1]&.last.to_s
      text.to_enum(:scan, /[*&]|[^*&]+/).map do
        part = Regexp.last_match
        following_character = text[part.end(0)] || following[0]
        category = part[0].match?(/\A[*&]\z/) && !following_character.to_s.match?(/\s/) ?
          Rouge::Token['Punctuation'] : token
        [category, part[0]]
      end
    end
  end

  def profile(token)
    if token == SEMANTIC_TOKEN
      :semantic
    elsif Rouge::Token['Keyword.Type'].matches?(token)
      :type
    elsif Rouge::Token['Name.Constant'].matches?(token) || Rouge::Token['Keyword.Constant'].matches?(token)
      :constant
    elsif Rouge::Token['Keyword'].matches?(token) || Rouge::Token['Operator'].matches?(token)
      :keyword
    elsif Rouge::Token['Literal'].matches?(token)
      :literal
    elsif Rouge::Token['Comment.Preproc'].matches?(token) || Rouge::Token['Comment.PreprocFile'].matches?(token)
      :preprocessor
    elsif Rouge::Token['Comment'].matches?(token)
      :comment
    elsif Rouge::Token['Name.Function'].matches?(token)
      :function
    elsif token == Rouge::Token['Name'] || Rouge::Token['Name.Variable'].matches?(token) ||
        Rouge::Token['Name.Label'].matches?(token)
      :variable
    else
      :default
    end
  end

  def font(token)
    PROFILE_FONTS.fetch(profile(token))
  end
end

class Pcre2SourceBlocks < Asciidoctor::Extensions::TreeProcessor
  def process(document)
    document.find_by(context: :listing).each do |block|
      if block.style != 'source' &&
          (Pcre2Source::SEMANTIC_ROLES.include?(block.style) || Rouge::Lexer.find(block.style))
        block.set_attr('language', block.style)
        block.style = 'source'
        block.set_attr('style', 'source')
        if document.basebackend?('html') && !block.attr?('subs')
          block.subs.map! { |sub| sub == :specialcharacters ? :highlight : sub }
        end
      end
      Pcre2Source.lexer(block.attr('language')) if block.style == 'source'
    end
    document
  end
end

# Base processor for short API macros such as `func:pcre2_match()`. Each macro
# becomes a monospaced quoted node with a semantic role; converters decide how
# that role is presented instead of the macro injecting backend-specific text.
class Pcre2ApiInlineMacro < Asciidoctor::Extensions::InlineMacroProcessor
  use_dsl
  format :short

  def process(parent, target, _attributes)
    create_inline parent, :quoted, target,
      type: :monospaced, attributes: {'role' => "#{@role} pcre2-api"}
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

class Pcre2MacroValueInlineMacro < Asciidoctor::Extensions::InlineMacroProcessor
  use_dsl
  named :macroval
  format :short
  match /macroval:((?:bytes,)?[A-Za-z0-9_]+)/

  DEFINITIONS = {
    'config.h.generic' => %w[
      HEAP_LIMIT MATCH_LIMIT MAX_NAME_COUNT MAX_NAME_SIZE
      MAX_VARLOOKBEHIND PARENS_NEST_LIMIT PCRE2GREP_BUFSIZE PCRE2GREP_MAX_BUFSIZE
    ],
    'pcre2_compile.c' => %w[MAX_GROUP_NUMBER MAX_REPEAT_COUNT],
    'pcre2_dfa_match.c' => %w[DFA_RECURSE_OVECTOR DFA_WORKSPACE_MIN],
    'pcre2_internal.h' => %w[
      MAX_CALLOUT_NUMBER MAX_OVECTOR_COUNT START_FRAMES_COUNT START_FRAMES_SIZE TABLES_LENGTH
    ],
    'pcre2_intmodedep.h' => %w[
      DEFAULT_MAX_PATTERN_SIZE_8_16 LOOKBEHIND_MAX MAX_MARK_8 MAX_MARK_16_32
    ],
    'pcre2_jit_compile.c' => %w[MACHINE_STACK_SIZE],
    'pcre2grep.c' => %w[
      PCRE2GREP_BINARY_CHECK_SIZE PCRE2GREP_DEFAULT_CAPTURE_MAX
      PCRE2GREP_MAX_ERRORS PCRE2GREP_MAXPATLEN_MIN
    ],
    'pcre2test.c' => %w[PCRE2TEST_DEFAULT_OVECCOUNT PCRE2TEST_LOOPREPEAT PCRE2TEST_PARENS_NEST_DEFAULT PCRE2TEST_REPLACE_BUFFSIZE]
  }.freeze

  def self.read_integers(file, names)
    source = File.read(File.expand_path("../src/#{file}", __dir__), encoding: Encoding::UTF_8)
    names.to_h do |name|
      pattern = /^#define #{name}\s+(\d+)[uU]?(?=\s|$)/
      matches = source.scan(pattern)
      raise "Expected one definition for #{name} in #{file}" unless matches.length == 1
      [name, Integer(matches.first.first, 0)]
    end
  end

  def self.format_integer(value)
    value.to_s.gsub(/(?<=\d)(?=(?:\d{3})+\z)/, ',')
  end

  def self.format_value(value, unit)
    return format_integer(value) if unit.nil?

    prefixes = ['', 'Ki', 'Mi', 'Gi']
    prefix_index = 0
    while value.positive? && prefix_index < prefixes.length - 1 && (value % 1024).zero?
      value /= 1024
      prefix_index += 1
    end
    suffix = prefix_index.zero? ? ' bytes' : "#{prefixes[prefix_index]}B"
    "#{format_integer(value)}#{suffix}"
  end

  def self.values
    @values ||= begin
      values = {}
      DEFINITIONS.each do |file, names|
        values.merge! read_integers(file, names)
      end
      values.freeze
    end
  end

  def process(parent, target, _attributes)
    unit, name = target.start_with?('bytes,') ? target.split(',', 2) : [nil, target]
    value = self.class.values.fetch(name) { raise "Unknown macroval:#{name}" }
    create_inline parent, :quoted, self.class.format_value(value, unit)
  end
end

# Install defaults and API macros for documents loaded with this extension.
Asciidoctor::Extensions.register do
  preprocessor Pcre2DocumentDefaults
  inline_macro Pcre2FunctionInlineMacro
  inline_macro Pcre2TypeInlineMacro
  inline_macro Pcre2ConstantInlineMacro
  inline_macro Pcre2ArgumentInlineMacro
  inline_macro Pcre2CharacterInlineMacro
  inline_macro Pcre2MacroValueInlineMacro
  tree_processor Pcre2SourceBlocks
  postprocessor Pcre2HtmlPostprocessor
end

# Token colours are part of our shared stylesheet, not Rouge's theme.
class Pcre2RougeAdapter < Asciidoctor::SyntaxHighlighter::RougeAdapter
  register_for 'rouge'

  def highlight(node, source, language, opts)
    @style ||= (style = opts[:style]) && style_available?(style) || DEFAULT_STYLE
    semantic = node.roles.find { |role| Pcre2Source::SEMANTIC_ROLES.include?(role) }
    highlighted = create_formatter(node, source, language, opts).format(Pcre2Source.tokens(source, semantic || language))
    if opts[:number_lines] && opts[:callouts]
      offset = highlighted.index(CodeCellStartTagCs)
      [highlighted, offset && offset + CodeCellStartTagCs.length]
    else
      highlighted
    end
  end

  def docinfo?(_location)
    false
  end

  def write_stylesheet?(_document)
    false
  end
end

module Pcre2ListLayout
  MIN_DLIST_WIDTH = 7
  MAX_DLIST_WIDTH = 26

  module_function

  def spaced?(node)
    return true if node.context == :dlist && node.items.any? { |terms, _| terms.length > 1 }

    items = node.context == :dlist ? node.items.map(&:last).compact : node.items
    items.any?(&:blocks?)
  end

  def label_width(node, terms)
    if node.attr?('width')
      value = node.attr('width')
      raise ArgumentError, "Invalid definition-list width: #{value.inspect}" unless value.match?(/\A[1-9]\d*\z/)

      return Integer(value, 10)
    end

    lengths = terms.map { |term| term.gsub(/<[^>]+>/, '').length }
    fitting = lengths.select { |length| length + 1 <= MAX_DLIST_WIDTH }
    fitting.length > lengths.length / 2 ? [fitting.max + 1, MIN_DLIST_WIDTH].max : MIN_DLIST_WIDTH
  end
end

class Pcre2HtmlConverter < Asciidoctor::Html5s::Converter
  register_for 'html5'

  def ulist(node, _opts = {})
    html_list(node, :ul)
  end

  def olist(node, _opts = {})
    html_list(node, :ol)
  end

  def dlist(node, _opts = {})
    return super if node.style == 'qanda'

    node.extend(Asciidoctor::Html5s::Converter::Helpers)
    terms_and_descriptions = node.items.map { |terms, description| [terms.map(&:text), description] }
    width = Pcre2ListLayout.label_width(node, terms_and_descriptions.flat_map(&:first))
    # Match the 20px tab and 10px minimum gap in document.css; round to double tabs.
    tabs = [((width * 9.6 + 10) / 40).ceil * 2, 2].max
    entries = terms_and_descriptions.map do |terms, description|
      term = node.html_tag(:dt, {}, node.html_tag(:span, {}, terms.join('<br>')))
      body = description ? node.html_tag(:dd, {}, node.print_item_content(description)) : ''
      node.html_tag(:div, {class: 'dlist-item'}, term + body)
    end.join
    attributes = list_attributes(node).merge(style: "--tab-width: #{tabs}")
    list = node.html_tag(:dl, attributes, entries)
    titled_list(node, list)
  end

  def html_list(node, tag)
    node.extend(Asciidoctor::Html5s::Converter::Helpers)
    checklist = tag == :ul && node.option?('checklist')
    entries = node.items.map do |item|
      content = node.print_item_content(item)
      if checklist && item.attr?(:checkbox)
        content = node.html_tag(:input, {type: 'checkbox', disabled: 'disabled',
          checked: ('checked' if item.attr?(:checked)), class: 'task-list-item-checkbox'}) + content
      end
      node.html_tag(:li, {id: item.id, class: item.role}, content)
    end.join
    attributes = list_attributes(node)
    attributes[:class] << 'task-list' if checklist
    if tag == :ol
      attributes.merge!(start: node.attr(:start), type: node.list_marker_keyword,
        reversed: ('reversed' if node.option?('reversed')))
    end
    list = node.html_tag(tag, attributes, entries)
    titled_list(node, list)
  end

  def list_attributes(node)
    {id: node.id, class: [node.style, node.role,
                         Pcre2ListLayout.spaced?(node) ? 'spaced' : 'compact']}
  end

  def titled_list(node, list)
    return list unless node.title?

    title = node.html_tag(:h6, {class: 'block-title'}, node.title)
    node.html_tag(:section, {class: "#{node.context} list-block"}, title + list)
  end
end

# Preserve PCRE2's established terminal layout while adding description-list,
# inline API, and C source presentation to Asciidoctor's manpage backend.
# `ESC_BS`, inherited from the stock converter, protects intended roff escapes
# until `manify` performs its final escape and fake-markup cleanup pass.
class Pcre2ManpageConverter < Asciidoctor::Converter::ManPageConverter
  PROTECTED_FONT_MARKER_RX = %r(</?#{Regexp.escape ESC_BS}f(B|I|R|\(C[RBI]|P)>)

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
    masked = text.gsub(%r(<pcre2-code>(.*?)</pcre2-code>)m) { protect.call(Regexp.last_match(1)) }
    masked = masked.gsub(/\bpcre2_[a-z0-9_]+(?:\(\))?|\bpcre2test\b/, &protect)
    super(masked).gsub(/\0(\d+)\0/) { protected[$1.to_i] }
  end

  def convert_dlist(node)
    return super if node.style == 'qanda'

    # PCRE2 uses traditional tagged paragraphs. `width` overrides the automatic
    # label width; fake inline-font tags are excluded from the measurement.
    result = []
    terms_and_descriptions = node.items.map do |terms, description|
      [terms.map(&:text), description]
    end
    width = Pcre2ListLayout.label_width(node, terms_and_descriptions.flat_map(&:first))
    spaced = Pcre2ListLayout.spaced?(node)
    # `.sp` retains normal separation from preceding prose, `.RS 4` indents the
    # whole list by four ens, and `.PD 0` suppresses inter-item paragraph
    # distance for compact single-term lists without additional blocks.
    result << '.sp'
    result << '.RS 4'
    result << (spaced ? '.PD' : '.PD 0')
    terms_and_descriptions.each do |terms, description|
      terms[0...-1].each do |term|
        result << ".TP #{width}\n#{manify term, whitespace: :normalize}"
        result << '.PD 0' if spaced
      end
      # `.TP width` emits a term followed by its indented description. The macro
      # resets adjustment, so restore ragged-right mode for every description.
      result << %(.TP #{width}
#{manify terms.last, whitespace: :normalize}
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
      result << '.PD' if spaced && terms.length > 1
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
    spaced = Pcre2ListLayout.spaced?(node)
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
    spaced = Pcre2ListLayout.spaced?(node)
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
    if node.type == :monospaced && (token = Pcre2Source.inline_token(node.roles, node.text))
      # Retain line-break points for type/constant names inside table cells.
      text = node.text
      text = text.gsub('_', "_#{ESC_BS}:") if node.parent.context == :table_cell && (node.roles & %w[type constant]).any?
      result = inline_font_run(text, Pcre2Source.font(token))
    else
      result = case node.type
      when :emphasis
        inline_font_run node.text, 'I'
      when :strong
        inline_font_run node.text, 'B'
      when :monospaced
        language = Pcre2Source.inline_language(node.roles)
        inline_font_run(language ? highlighted_source(node.text, language) : node.text, '(CR')
      when :single
        %[<#{ESC_BS}(oq>#{node.text}</#{ESC_BS}(cq>]
      when :double
        %[<#{ESC_BS}(lq>#{node.text}</#{ESC_BS}(rq>]
      else
        node.text
      end
    end
    case_sensitive_heading(result, node)
  end

  def case_sensitive_heading(text, node)
    # Stock uppercase handling only protects CR text, not newly italic/bold code.
    if node.type == :monospaced && node.parent.context == :section && node.parent.level <= 1
      "<pcre2-code>#{text}</pcre2-code>"
    else
      text
    end
  end

  def inline_font_run(text, font)
    text.empty? ? '' : %(<#{ESC_BS}f#{font}>#{text}</#{ESC_BS}fP>)
  end

  # Asciidoctor finishes nested inline substitutions after the outer converter
  # callback. Flatten the resulting protected font markers before stock manify
  # turns them into roff escapes.
  def manify(text, options = {})
    super flatten_inline_fonts(text.gsub(%r(</?pcre2-code>), '')), options
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
        font = match[1]
        font = "(C#{font}" if %w[R B I].include?(font) && fonts.any? { |parent| parent.start_with?('(C') }
        fonts << font
      end
      offset = match.end(0)
    end
    segment = text[offset..]
    result << (fonts.empty? ? segment : inline_font_run(segment, fonts.last))
    raise 'Unbalanced protected roff font markers' unless fonts.empty?

    result.join
  end

  def convert_literal(node)
    content = node.content
    if (node.roles & Pcre2Source::SEMANTIC_ROLES).any?
      content = inline_font_run(content, Pcre2Source.font(Pcre2Source::SEMANTIC_TOKEN))
    end
    convert_preformatted node, manify(content, whitespace: :preserve)
  end

  def convert_listing(node)
    language = node.attr('language')
    content = if Pcre2Source::SEMANTIC_ROLES.include?(language) ||
        (node.roles & Pcre2Source::SEMANTIC_ROLES).any?
      manify(inline_font_run(node.content, Pcre2Source.font(Pcre2Source::SEMANTIC_TOKEN)), whitespace: :preserve)
    elsif node.style == 'source'
      convert_source(node)
    else
      manify(node.content, whitespace: :preserve)
    end
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

  # Callout blocks contain protected roff escapes; keep their existing path.
  def convert_source(node)
    return manify(node.content, whitespace: :preserve) if node.content.include? ESC_BS

    manify highlighted_source(node.content, node.attr('language')), whitespace: :preserve
  end

  def highlighted_source(source, language)
    Pcre2Source.tokens(CGI.unescapeHTML(source), language).chunk { |token, _| Pcre2Source.font(token) }.map do |font, tokens|
      apply_roff_font(tokens.map(&:last).join, font)
    end.join
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

end

module Pcre2HtmlOutput
  DOCUMENT_NAMES = Dir.glob(File.expand_path('../doc/*.adoc', __dir__)).map { |path| File.basename(path, '.adoc') }.freeze

  module_function

  def metadata(document)
    version = document.attr('mansource', "PCRE2 #{Pcre2Maintenance::CURRENT_RELEASE}")
    kind = document.doctype == 'manpage' || document.attr('docname') == 'index' ?
      'manual' : 'documentation'
    result = "PCRE2 #{CGI.escapeHTML(version.delete_prefix('PCRE2 '))} #{kind}"
    if (revision = document.attr('revdate'))
      date = Date.parse(revision)
      result += %(; updated <time datetime="#{date.iso8601}">#{date.strftime('%-d %b %Y')}</time>)
    end
    %(<p class="document-meta">#{result}</p>)
  end

  def picture(kind, alt)
    %(<picture><source media="(prefers-color-scheme: dark)" srcset="./images/pcre2-#{kind}-dark.svg">) +
      %(<img src="./images/pcre2-#{kind}-light.svg" alt="#{alt}"></picture>)
  end

  def navbar(document)
    home = %(<a class="offline-home" href="https://www.pcre2.org/"><span aria-hidden="true">&#x1f310;&#xfe0e;</span> PCRE2 homepage</a>)
    if document.attr('docname') == 'index'
      %(<header class="offline-nav offline-hero"><nav>#{picture('header', 'PCRE2: Perl-Compatible Regular Expressions')}#{home}</nav></header>)
    else
      %(<header class="offline-nav"><nav><a class="offline-index" href="./index.html">Index</a>#{home}</nav></header>)
    end
  end

  def normalize_links(tree, document)
    site = document.attr?('pcre2-site')
    names = DOCUMENT_NAMES + document.attr('pcre2-documents', '').split(',')
    current = document.attr('docname')
    tree.css('a[href]').each do |anchor|
      href = anchor['href']
      next if href.empty? || href.start_with?('#', '/', '?') || href.match?(/\A[a-z][a-z\d+.-]*:/i)

      path, separator, tail = href.partition(/[?#]/)
      if (match = /\A((?:\.\/)?(?:\.\.\/)?)([^\/]+)\.html\z/.match(path)) && names.include?(match[2])
        prefix, name = match[1], match[2]
        path = if site
          prefix = current == 'index' ? './' : './../'
          name == current ? './' : "#{prefix}#{name == 'index' ? '' : "#{name}/"}"
        else
          "#{prefix}#{name}.html"
        end
      end
      path = "./#{path}" unless path.start_with?('./')
      anchor['href'] = path + separator + tail
    end
  end

  def highlight_inline(tree, document)
    return unless document.syntax_highlighter&.name == 'rouge'

    highlight_source(tree)
  end

  def highlight_source(tree)
    tree.css('code[class]').each do |code|
      next if code.ancestors('pre').any?
      next if code['data-lang'] && code['class'].split.include?('rouge')
      next if Pcre2Source.inline_token(code['class'].split, code.text)
      language = Pcre2Source.inline_language(code['class'].split)
      next unless language

      tokens = Pcre2Source.tokens(code.text, language)
      index = offset = 0
      # Keep explicit emphasis, links and other inline markup; distribute tokens
      # over their text nodes instead of replacing the whole code element.
      code.xpath('.//text()').each do |text_node|
        annotated = text_node.ancestors('code[class]').any? do |ancestor|
          ancestor != code && Pcre2Source.inline_token(ancestor['class'].split, ancestor.text)
        end
        remaining = text_node.text.length
        while remaining.positive?
          token, text = tokens.fetch(index)
          length = [remaining, text.length - offset].min
          value = text[offset, length]
          replacement = Nokogiri::XML::Text.new(value, tree.document)
          unless token.shortname.empty? || annotated
            span = Nokogiri::XML::Node.new('span', tree.document)
            span['class'] = token.shortname
            span.add_child(replacement)
            replacement = span
          end
          text_node.add_previous_sibling(replacement)
          remaining -= length
          offset += length
          index, offset = index + 1, 0 if offset == text.length
        end
        text_node.remove
      end
      code.add_class('rouge')
      code['data-lang'] = language
    end
  end

  def wrap_token(node, token)
    span = Nokogiri::XML::Node.new('span', node.document)
    span['class'] = token.shortname
    span.add_child(node.children)
    node.add_child(span)
  end

  def annotate_semantics(tree)
    tree.css('code[class]').each do |code|
      token = Pcre2Source.inline_token(code['class'].split, code.text)
      token ||= Pcre2Source::SEMANTIC_TOKEN if Pcre2Source::SEMANTIC_ROLES.include?(code['data-lang'])
      next unless token

      code.add_class('rouge')
      wrap_token(code, token) unless code.at_css(".#{token.shortname}")
    end
    tree.css('.listing-block pre, .literal-block pre').each do |pre|
      block = pre.ancestors.find { |parent| parent['class'].to_s.split.any? { |name| %w[listing-block literal-block].include?(name) } }
      next unless block && (block['class'].split & Pcre2Source::SEMANTIC_ROLES).any?

      pre.add_class('rouge')
      wrap_token(pre, Pcre2Source::SEMANTIC_TOKEN) unless pre.at_css('.pcre2-semantic')
    end
  end

  def annotate_characters(tree)
    links = []
    tree.css('code.character.pcre2-api').each_with_index do |code, index|
      value = Integer(code.text.delete_prefix('U+').delete_prefix('0x'), 16)
      label = format('U+%04X', value)
      glyph = ''
      if value > 0x10ffff
        name = 'Outside Unicode range'
        label = code.text
      elsif (0xd800..0xdfff).cover?(value)
        name = 'Surrogate code point'
      else
        character = value.chr(Encoding::UTF_8)
        name = Unicode::Name.readable(character)
        unless character.match?(/[\p{C}\p{Z}\p{Default_Ignorable_Code_Point}]/)
          glyph = character.match?(/\p{M}/) ? "\u25cc#{character}" : character
        end
      end
      card = Nokogiri::XML::Node.new('span', tree.document)
      card['class'] = 'character-card'
      card['id'] = "pcre2-character-#{index + 1}"
      card['role'] = 'tooltip'
      card['aria-label'] = "#{name} (#{label})"
      card['aria-hidden'] = 'true'
      card['data-pagefind-ignore'] = ''
      {
        'glyph' => ['data-character', glyph],
        'name' => ['data-character-name', name],
        'codepoint' => ['data-codepoint', label],
      }.each do |kind, (attribute, value)|
        part = Nokogiri::XML::Node.new('span', tree.document)
        part['class'] = "character-#{kind}"
        part[attribute] = value
        card.add_child(part)
      end

      wrapper = Nokogiri::XML::Node.new('span', tree.document)
      wrapper['class'] = 'character-info'
      # Suppress any enclosing link's native tooltip.
      wrapper['title'] = ''
      if (link = code.ancestors('a').first)
        unless links.include?(link)
          wrapper.add_class('character-link-focus')
          link['aria-describedby'] = [link['aria-describedby'], card['id']].compact.join(' ')
          links << link
        end
      else
        wrapper['tabindex'] = '0'
        wrapper['aria-describedby'] = card['id']
      end
      code.replace(wrapper)
      wrapper.add_child(code)
      wrapper.add_child(card)
    end
  end

  def process(document, output)
    full = output.start_with?('<!DOCTYPE')
    tree = full ? Nokogiri::HTML5.parse(output) : Nokogiri::HTML5.fragment(output)
    highlight_inline(tree, document)
    annotate_semantics(tree)
    normalize_links(tree, document)
    tree.css('.toc-list a').each { |link| link['title'] ||= link.text }
    annotate_characters(tree)
    return tree.to_html unless full || document.attr?('pcre2-site')

    container = full ? tree.at_css('body') : tree
    title = full ? container.at_xpath('./header/h1') : nil
    content = full ? container.at_css('#content') : container
    raise 'Standalone HTML has no content container' unless content

    toc = container.at_css('[role="doc-toc"]')
    toc&.unlink
    article = Nokogiri::XML::Node.new('article', tree.document)
    article['class'] = 'prose'
    article['id'] = 'content' if full
    content.children.each { |child| article.add_child(child) }
    if full
      container.at_xpath('./header')&.remove
      content.remove
      container.children.each { |child| article.add_child(child) }
    end
    container.children.remove

    header = Nokogiri::XML::Node.new('header', tree.document)
    header['class'] = 'document-title'
    if title
      header.add_child(title)
    elsif document.header?
      heading = Nokogiri::XML::Node.new('h1', tree.document)
      heading.content = document.attr('pcre2-title', document.doctitle)
      header.add_child(heading)
    end
    header.add_child(Nokogiri::HTML5.fragment(metadata(document)))
    layout = Nokogiri::XML::Node.new('div', tree.document)
    layout['class'] = 'document-layout'
    layout.add_child(toc) if toc
    layout.add_child(article)
    if full
      head = tree.at_css('head')
      head.add_child(Nokogiri::HTML5.fragment(
        %(<link rel="stylesheet" href="./css/standalone.css">) +
        %(<meta name="color-scheme" content="light dark">) +
        %(<meta name="theme-color" media="(prefers-color-scheme: light)" content="#0085cc">) +
        %(<meta name="theme-color" media="(prefers-color-scheme: dark)" content="#33b8ff">)))
      main = Nokogiri::XML::Node.new('main', tree.document)
      main['class'] = 'document-page'
      main.add_child(header)
      main.add_child(layout)
      container.add_child(Nokogiri::HTML5.fragment(navbar(document)))
      container.add_child(main)
      container.add_child(Nokogiri::HTML5.fragment('<footer class="offline-footer"></footer>'))
    else
      container.add_child(header)
      container.add_child(layout)
    end
    # Include links introduced by the standalone shell in the same policy.
    normalize_links(tree, document)
    tree.to_html
  end
end

module Pcre2AsciidocConverter
  module_function

  def copy_html_assets(directory)
    assets = File.expand_path('../doc/html', __dir__)
    return if File.expand_path(directory) == assets

    FileUtils.mkdir_p(directory)
    FileUtils.cp_r(%w[css images].map { |name| File.join(assets, name) }, directory)
  end

  # Convert AsciiDoc sources into their HTML and manpage outputs.
  def run(inputs, embedded: false, output_dir: nil)
    raise ArgumentError, 'Embedded HTML requires an output directory' if embedded && !output_dir

    logger = Asciidoctor::LoggerManager.logger
    logger.level = Logger::Severity::DEBUG
    names = inputs.map { |input| File.basename(input, '.adoc') }.join(',')

    inputs.each do |input|
      base = File.basename(input, '.adoc')
      directory = File.dirname(input)
      if embedded
        Asciidoctor.convert_file input, backend: 'html5', safe: :unsafe, mkdirs: true,
          header_footer: false,
          attributes: {'notitle' => '', 'pcre2-site' => '', 'relfilesuffix' => '.html',
                       'pcre2-title' => base == 'index' ? 'Manual pages' : base,
                       'pcre2-documents' => names,
                       'relfileprefix' => base == 'index' ? '' : '../'},
          to_file: File.join(output_dir, "#{base}.html")
        next
      end

      unless base == 'index'
        section = File.read(input, encoding: Encoding::UTF_8)[/^:manvolnum: (.+)$/, 1]
        raise "Invalid manpage section in #{input}" unless section

        Asciidoctor.convert_file input, backend: 'manpage', safe: :unsafe, mkdirs: true,
          to_file: File.join(directory, 'man', "#{base}.#{section}")
      end
      Asciidoctor.convert_file input, backend: 'html5', safe: :unsafe, mkdirs: true,
        attributes: {'pcre2-documents' => names},
        to_file: File.join(output_dir || File.join(directory, 'html'), "#{base}.html")
    end

    raise 'Asciidoctor generated warnings' if logger.max_severity && logger.max_severity >= Logger::Severity::WARN

    unless embedded
      directories = inputs.map { |input| output_dir || File.join(File.dirname(input), 'html') }.uniq
      directories.each { |directory| copy_html_assets(directory) }
    end
  end
end

if $PROGRAM_NAME == __FILE__
  options = {}
  OptionParser.new do |parser|
    parser.banner = "Usage: #{$PROGRAM_NAME} [--embedded --output-dir DIR] [input.adoc ...]"
    parser.on('--embedded', 'Render website HTML fragments, without manpages') { options[:embedded] = true }
    parser.on('--output-dir DIR', 'Write HTML to DIR') { |directory| options[:output_dir] = directory }
  end.parse!
  inputs = ARGV.empty? ? Dir.glob('*.adoc').sort : ARGV
  abort 'No AsciiDoc sources found' if inputs.empty?
  Pcre2AsciidocConverter.run(inputs, **options)
end