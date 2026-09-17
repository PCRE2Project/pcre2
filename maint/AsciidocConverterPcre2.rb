# frozen_string_literal: true

require 'asciidoctor/converter/manpage'
require 'asciidoctor/converter/html5'

class Pcre2ManpageConverter < Asciidoctor::Converter::ManPageConverter
  register_for 'manpage'

  def manify(str, opts = {})
    super.gsub('\\(cq', "'")
  end

  def convert_section(node)
    # Stock manpage conversion uppercases level-one titles after AsciiDoc has
    # parsed them. Preserve inline code and backslash sequences while matching
    # the historical all-caps rendering of ordinary heading text.
    macro = node.level > 1 ? 'SS' : 'SH'
    title = node.level > 1 ? node.captioned_title : uppercase_title(node.title)
    %(.#{macro} "#{manify title}"
#{node.content})
  end

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
    # Stock Asciidoctor emits a leading `.sp` and places each entry inside
    # `.RS 4` / `.RE`. PCRE2 uses traditional `.TP 10` tagged paragraphs so
    # terms and descriptions start at the normal manpage margins.
    result = []
    indent = node.attr 'indent', 10
    node.items.each do |terms, description|
      result << %(.TP #{indent}
#{manify terms.map {|term| term.text.sub(/::\z/, '') }.join(', '), whitespace: :normalize})
      if description
        result << (manify description.text, whitespace: :normalize) if description.text?
        result << description.content if description.blocks?
      end
    end
    result << '.PP'
    result.join "\n"
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
    %(.sp
.nf
.fam C
#{manify node.content, whitespace: :preserve}
.fam
.fi)
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
    # width; non-source listings retain the stock implementation.
    return super unless node.style == 'source'

    result = []
    result << %(.sp
.B #{manify node.captioned_title}
.br) if node.title?
    result << %(.sp
.nf
.fam C
#{manify node.content, whitespace: :preserve}
.fam
.fi)
    # The `wide` role is used for pcre2demo: extend the source left by seven
    # ens beyond the normal margin, matching the historical troff presentation.
    result[1] = ".RS -7\n#{result[1]}" if node.role == 'wide'
    result.join "\n"
  end
end

class Pcre2Html5Converter < Asciidoctor::Converter::Html5Converter
  register_for 'html5'

  def convert_literal(node)
    # Stock HTML renders every literal block as `literalblock > pre`. Preserve
    # the dedicated synopsis markup and render other literal rows as indented
    # prose, to match their manpage semantics.
    return convert_synopsis(node) if node.style == 'synopsis'

    convert_formatted node
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

  def convert_formatted(node)
    # Stock HTML would place this literal block in `<pre>`. These rows are
    # formatted prose, so preserve declared substitutions and line breaks in
    # an indented paragraph instead; `indent` defaults to two character cells.
    content = node.content
    indent = node.attr('indent', 2)
    %(<div class="paragraph">
  <p style="margin-left: #{indent}ch">#{content.gsub("\n", "<br>\n")}</p>
</div>)
  end
end