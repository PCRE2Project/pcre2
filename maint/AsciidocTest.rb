#! /usr/bin/env ruby
# frozen_string_literal: true

require 'minitest/autorun'
require 'tmpdir'
require_relative 'AsciidocConverterPcre2'

class Pcre2AsciidocTest < Minitest::Test
  def setup
    @previous_logger = Asciidoctor::LoggerManager.logger
    Asciidoctor::LoggerManager.logger = Asciidoctor::MemoryLogger.new
  end

  def teardown
    Asciidoctor::LoggerManager.logger = @previous_logger
  end

  def html(source)
    Asciidoctor.convert(source, backend: 'html5', safe: :unsafe, header_footer: false)
  end

  def test_compact_lists_and_nested_list_classes
    result = html(<<~ADOC)
      * First
      ** Nested
      * Second
    ADOC
    assert_match(/<ul class="spaced">/, result)
    assert_match(/<ul class="compact">/, result)
    assert_equal 3, result.scan('<li>').length
  end

  def test_compact_ordered_list_preserves_numbering_role_and_id
    result = html(<<~ADOC)
      [[steps]]
      [loweralpha,start=3,role=steps]
      . First
      . Second
    ADOC
    assert_match(/<ol id="steps" class="loweralpha steps compact" start="3" type="a">/, result)
  end

  def test_blocks_make_all_list_items_spaced
    result = html(<<~ADOC)
      . First
      +
      Another paragraph.
      . Second
    ADOC
    assert_match(/<ol class="arabic spaced">/, result)
    assert_includes result, '<p>Another paragraph.</p>'
  end

  def test_titled_checklist_preserves_title_and_state
    result = html(<<~ADOC)
      .Tasks
      * [x] Done
      * [ ] Pending
    ADOC
    assert_includes result, '<h6 class="block-title">Tasks</h6>'
    assert_match(/<input type="checkbox" disabled="disabled" checked="checked" class="task-list-item-checkbox">/, result)
    assert_match(/<input type="checkbox" disabled="disabled" class="task-list-item-checkbox">/, result)
  end

  def test_definition_list_groups_aliases_and_honours_width
    result = html(<<~ADOC)
      [.options,width=12]
      one::
      two:: Description.
      a_very_long_option_name:: Another description.
    ADOC
    assert_match(/<dl class="options compact" style="--term-width: 6.0em">/, result)
    assert_includes result, '<div class="dlist-item"><dt>one, two</dt><dd>Description.</dd></div>'
    assert_includes result, '<div class="dlist-item long-term"><dt>a_very_long_option_name</dt>'
  end

  def test_definition_list_blocks_are_spaced
    result = html(<<~ADOC)
      term:: Description.
      +
      More detail.
      other:: Another description.
    ADOC
    assert_match(/<dl class="spaced"/, result)
    assert_includes result, '<dd><p>Description.</p><p>More detail.</p></dd>'
  end

  def test_question_and_answer_role
    result = html("[qanda]\nQuestion?:: Answer.")
    assert_match(/<dl class="qanda compact".*role="doc-qna"/, result)
  end

  def test_shared_compactness_preserves_manpage_spacing
    compact = Asciidoctor.load("* First\n* Second", backend: 'manpage')
    spaced = Asciidoctor.load("* First\n+\nMore detail.\n* Second", backend: 'manpage')
    converter = Pcre2ManpageConverter.new('manpage')
    assert_equal 1, converter.convert_ulist(compact.blocks.first).scan(/^\.sp$/).length
    assert_equal 3, converter.convert_ulist(spaced.blocks.first).scan(/^\.sp$/).length
  end

  def test_owned_stylesheet_and_semantic_highlighting
    result = Asciidoctor.convert(<<~ADOC, backend: 'html5', safe: :unsafe, header_footer: true)
      = Test

      type:PCRE2_SIZE arg:offset const:PCRE2_UTF func:pcre2_match()

      [source,c]
      ----
      int value = 42;
      ----
    ADOC
    assert_match(/<link rel="stylesheet" href="(?:\.\/)?css\/pcre2.css">/, result)
    assert_includes result, 'offline-manual'
    assert_includes result, '<code class="type">PCRE2_SIZE</code>'
    assert_includes result, '<code class="parameter">offset</code>'
    assert_includes result, '<span class="kt">int</span>'
    refute_includes result, 'rouge-github.css'
    refute_includes result, '<style>'
    refute_includes result, 'fonts.googleapis'
  end

  def with_sources
    Dir.mktmpdir('pcre2-asciidoc-') do |directory|
      index = File.join(directory, 'index.adoc')
      manual = File.join(directory, 'manual.adoc')
      File.write(index, "= Manuals\n\n[[contents]]\n== Contents\n\nxref:manual.adoc#details[Manual]\n", encoding: 'UTF-8')
      File.write(manual, <<~ADOC, encoding: 'UTF-8')
        = manual(3)
        :doctype: manpage
        :manname: manual
        :manpurpose: Test manual
        :manvolnum: 3
        :toc:

        [[details]]
        == Details

        xref:index.adoc#contents[All manuals] and <<details>>.
        A note.footnote:[Footnote text.]
      ADOC
      yield directory, index, manual
    end
  end

  def test_batch_fragments_keep_relative_directory_xrefs_toc_and_footnotes
    with_sources do |directory, index, manual|
      output = File.join(directory, 'site')
      Pcre2AsciidocConverter.run([index, manual], embedded: true, output_dir: output)
      index_html = File.read(File.join(output, 'index.html'))
      manual_html = File.read(File.join(output, 'manual.html'))
      assert_includes index_html, 'href="manual/#details"'
      assert_includes manual_html, 'href="../#contents"'
      assert_includes manual_html, 'href="#details"'
      assert_includes manual_html, 'role="doc-toc"'
      assert_includes manual_html, 'Footnote text.'
      refute_match(/<!DOCTYPE|<html|<head|<h1/, manual_html)
      refute Dir.exist?(File.join(directory, 'man'))
      refute Dir.exist?(File.join(output, 'css'))
    end
  end

  def test_full_documents_keep_flat_xrefs_and_bundle_assets
    with_sources do |directory, index, manual|
      Pcre2AsciidocConverter.run([index, manual])
      output = File.join(directory, 'html')
      result = File.read(File.join(output, 'manual.html'))
      assert_includes result, '<!DOCTYPE html>'
      assert_includes result, 'href="index.html#contents"'
      assert File.file?(File.join(directory, 'man/manual.3'))
      shared = File.read(File.expand_path('../pages/static/css/document.css', __dir__))
      css = File.read(File.join(output, 'css/pcre2.css'))
      assert css.start_with?(shared)
      assert_includes css, '.offline-manual'
      fonts = File.expand_path('../pages/static/fonts', __dir__)
      Dir.glob(File.join(fonts, '*.{woff2,txt}')).each do |font|
        assert_equal File.binread(font), File.binread(File.join(output, 'fonts', File.basename(font)))
      end
    end
  end

  def test_missing_fragment_output_directory_is_an_error
    assert_raises(ArgumentError) { Pcre2AsciidocConverter.run([], embedded: true) }
  end

  def test_warnings_fail_batch_conversion
    Dir.mktmpdir('pcre2-asciidoc-') do |directory|
      input = File.join(directory, 'index.adoc')
      File.write(input, "= Test\n\ninclude::missing.adoc[]\n")
      error = assert_raises(RuntimeError) { Pcre2AsciidocConverter.run([input]) }
      assert_equal 'Asciidoctor generated warnings', error.message
    end
  end
end
