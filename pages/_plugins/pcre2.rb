# frozen_string_literal: true

require 'kramdown'
require 'kramdown-parser-gfm'
require 'asciidoctor'
require File.expand_path('../../maint/AsciidocConverterPcre2', __dir__) unless defined?(Pcre2Source)

class Kramdown::Parser::Pcre2GFM < Kramdown::Parser::GFM
  FENCED_CODEBLOCK_MATCH = /^[ ]{0,3}(([~`])\2{2,})[ \t]*(([^ \t\n`]+)(?:[ \t]+[^\n`]*)?)?[ \t]*\n(.*?)^[ ]{0,3}\1\2*[ \t]*(?:\n|\z)/m
  HEADER_TABLE = /^([^\n]*\|[^\n]*\n)([ \t]*\|?[ \t]*:?-+:?[ \t]*(?:\|[ \t]*:?-+:?[ \t]*)*\|?[ \t]*\n)(?=\n|\z)/

  def initialize(source, options)
    super
    @span_parsers << :pcre2_autolink
  end

  def parse_pcre2_autolink
    url = @src.matched.sub(/[.,:;!?]+\z/, '')
    url = url.chop while url.end_with?(')') && url.count(')') > url.count('(')
    @src.pos += url.bytesize
    if [:a, :img].include?(@tree.type) || @stack.any? { |element, _type| [:a, :img].include?(element.type) }
      add_text(url)
      return true
    end
    element = Kramdown::Element.new(:a, nil, {'href' => url}, location: @src.current_line_number)
    add_text(url, element)
    @tree.children << element
    true
  end
  define_parser(:pcre2_autolink, %r{https?://(?:\[[[:xdigit:]:.]+\]|[^\s<>\[\]]+)[^\s<>\[\]]*}, 'h')

  def parse_codeblock_fenced
    indent = @src.check(/^[ ]{0,3}/).length
    result = super
    @tree.children.last.value.gsub!(/^ {0,#{indent}}/, '') if result && indent.positive?
    result
  end

  def parse_table
    return super unless after_block_boundary? && (source = @src.check(HEADER_TABLE))

    # Kramdown requires a body; GFM permits a table containing only its header.
    environment = save_env
    begin
      @src = Kramdown::Utils::StringScanner.new(source + "| |\n\n", @src.current_line_number)
      raise 'Could not parse header-only Markdown table' unless super
      @tree.children.last.children.reject! { |element| element.type == :tbody }
    ensure
      restore_env(environment)
    end
    @src.pos += source.bytesize
    true
  end
end

module Pcre2Pages
  PROJECT_PAGES = %w[AUTHORS LICENCE SECURITY SUPPORT-LIFECYCLE].freeze

  class MarkdownHtml < Kramdown::Converter::Html
    def source_html(text, language, token = nil)
      tokens = token ? [[token, text]] : Pcre2Source.tokens(text, language)
      Rouge::Formatters::HTML.new.format(tokens)
    end

    def convert_codeblock(element, indent)
      attributes = element.attr.dup
      language = (element.options[:lang] || extract_code_language!(attributes)).to_s.split.first.to_s
      styled = !Pcre2Source::PLAIN_LANGUAGES.include?(language) || Pcre2Source::SEMANTIC_ROLES.include?(language)
      attributes['class'] = [attributes['class'], styled ? 'rouge' : nil].compact.join(' ')
      attributes.delete('class') if attributes['class'].empty?
      code_attributes = language.empty? ? {} : {'class' => "language-#{language}", 'data-lang' => language}
      "#{' ' * indent}<pre#{html_attributes(attributes)}><code#{html_attributes(code_attributes)}>" \
        "#{source_html(element.value, language)}</code></pre>\n"
    end

    def convert_codespan(element, _indent)
      attributes = element.attr.dup
      roles = attributes.fetch('class', '').split
      token = Pcre2Source.inline_token(roles, element.value)
      language = extract_code_language(attributes) || Pcre2Source.inline_language(roles)
      if token || language
        attributes['class'] = [attributes['class'], 'rouge'].compact.join(' ')
        attributes['data-lang'] = language if language
      end
      format_as_span_html('code', attributes, source_html(element.value, language, token))
    end
  end

  class Generator < Jekyll::Generator
    safe false

    def generate(site)
      root = site.config.fetch('pcre2_root', File.expand_path('..', site.source))
      documents = Dir.glob(File.join(site.config.fetch('pcre2_doc_dir', File.join(root, 'doc')), '*.adoc')).sort
      raise 'No AsciiDoc manuals found' if documents.empty?

      names = documents.map { |path| File.basename(path, '.adoc') }.join(',')
      original_logger = Asciidoctor::LoggerManager.logger
      logger = Asciidoctor::MemoryLogger.new
      Asciidoctor::LoggerManager.logger = logger
      begin
        documents.each do |path|
          name = File.basename(path, '.adoc')
          index = name == 'index'
          title = index ? 'Manual pages' : name
          content = Asciidoctor.convert_file(
            path, backend: 'html5', safe: :unsafe, header_footer: false, to_file: false,
            attributes: {'notitle' => '', 'pcre2-site' => '', 'relfilesuffix' => '.html',
                         'pcre2-title' => title, 'pcre2-documents' => names,
                         'relfileprefix' => index ? '' : '../'}
          )
          add_page(site, "doc/#{name}.html", index ? '/doc/' : "/doc/#{name}/", title, content,
                   'layout' => 'manual', 'manual_index' => index, 'render_with_liquid' => false)
        end
        if logger.max_severity && logger.max_severity >= Logger::Severity::WARN
          logger.messages.each { |message| Jekyll.logger.error 'AsciiDoc:', message[:message].to_s }
          raise 'Asciidoctor generated warnings'
        end
      ensure
        Asciidoctor::LoggerManager.logger = original_logger
      end
      {'project' => PROJECT_PAGES, 'guide' => %w[README NON-AUTOTOOLS-BUILD]}.each do |directory, names|
        names.each do |name|
          source = File.read(File.join(root, "#{name}.md"), encoding: 'UTF-8')
          title = /\A(?:# ([^\n]+)\n+|([^\n]+)\n=+\n+)/.match(source)
          raise "Missing Markdown title: #{name}" unless title

          add_page(site, "#{directory}/#{name}.md", "/#{directory}/#{name.downcase}/",
                   title[1] || title[2], source[title.end(0)..],
                   'layout' => directory == 'guide' ? 'guide' : 'default',
                   'pcre2_project' => directory == 'project', 'render_with_liquid' => false)
        end
      end
      assets = File.join(root, 'doc/html')
      files = [File.join(assets, 'css/document.css'), *Dir.glob(File.join(assets, 'images/*.svg'))]
      files.each do |path|
        relative = path.delete_prefix(assets + File::SEPARATOR)
        site.static_files.reject! { |file| file.relative_path.sub(%r{\A/}, '') == relative }
        site.static_files << Jekyll::StaticFile.new(site, assets, File.dirname(relative), File.basename(relative))
      end
    end

    def add_page(site, filename, permalink, title, content, data)
      page = Jekyll::PageWithoutAFile.new(site, site.source, File.dirname(filename), File.basename(filename))
      page.data.merge!({'title' => title, 'permalink' => permalink, 'layout' => 'default'}.merge(data))
      page.content = content
      site.pages << page
    end
  end
end

class Jekyll::Converters::Markdown::Pcre2
  def initialize(config)
    @options = config.fetch('kramdown', {})
  end

  def convert(content)
    document = Kramdown::Document.new(content, @options)
    output, warnings = Pcre2Pages::MarkdownHtml.convert(document.root, document.options)
    literal_references, messages = (document.warnings + warnings).partition do |message|
      message.match?(/\ANo link definition for link ID '.*' found on line \d+\z/)
    end
    literal_references.each { |message| Jekyll.logger.warn 'Markdown (literal brackets):', message }
    unless messages.empty?
      messages.each { |message| Jekyll.logger.error 'Markdown:', message }
      raise 'Kramdown generated warnings'
    end
    tree = Nokogiri::HTML5.fragment(output)
    Pcre2HtmlOutput.highlight_source(tree)
    Pcre2HtmlOutput.annotate_semantics(tree)
    tree.to_html
  end
end

Jekyll::Hooks.register :pages, :post_render do |page|
  next unless page.data['pcre2_project']

  tree = Nokogiri::HTML5(page.output)
  tree.css('main a[href]').each do |link|
    href = link['href']
    next if href.start_with?('#', '/') || href.match?(/\A[a-z][a-z\d+.-]*:/i)

    match = %r{\A\./(AUTHORS|LICENCE|SECURITY|SUPPORT-LIFECYCLE)\.md([?#].*)?\z}.match(href)
    raise "Could not adjust project link: #{href}" unless match

    link['href'] = "../#{match[1].downcase}/#{match[2]}"
  end
  page.output = tree.to_html
end
