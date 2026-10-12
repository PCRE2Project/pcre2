#! /usr/bin/env ruby
# frozen_string_literal: true

# Prepare generated files before "make dist". Pass "doc" to update only docs.
# This script must be run before "make dist"; no arguments other than "doc"
# are accepted.

require 'fileutils'
require 'open3'

require_relative 'UpdateCommon'
require_relative 'AsciidocDemo'
require_relative 'AsciidocIndex'

module Pcre2UpdateAlways
  extend Pcre2Maintenance

  # Use groff's conventional terminal width for rendered manpage text.
  MANPAGE_WIDTH = 78

  # Concatenate these section-three manuals into pcre2.txt, omitting the
  # individual function pages and pcre2demo as the generated header explains.
  MANUALS = %w[
    pcre2
    pcre2api
    pcre2build
    pcre2callout
    pcre2compat
    pcre2convert
    pcre2jit
    pcre2limits
    pcre2matching
    pcre2partial
    pcre2pattern
    pcre2perform
    pcre2posix
    pcre2sample
    pcre2serialize
    pcre2syntax
    pcre2unicode
  ].freeze

  # Detrail these text files, but do not touch test data because trailing
  # spaces can be significant. Configure-generated files are also omitted.
  TEXT_FILES = %w[
    AUTHORS.md
    BUILD.bazel
    CMakeLists.txt
    ChangeLog
    HACKING
    INSTALL
    LICENCE.md
    MODULE.bazel
    Makefile.am
    NEWS
    NON-AUTOTOOLS-BUILD
    PerlTest.sh
    README
    RunGrepTest
    RunGrepTest.py
    RunPerlTest
    RunTest
    RunTest.py
    SECURITY.md
    SUPPORT-LIFECYCLE.md
    build.zig
    build.zig.zon
    cmake/*.cmake
    cmake/*.in
    cmake/COPYING-CMAKE-SCRIPTS
    configure.ac
    doc/html/*
    doc/man/*
    doc/p*
    libpcre2-16.pc.in
    libpcre2-32.pc.in
    libpcre2-8.pc.in
    libpcre2-posix.pc.in
    m4/ax_pthread.m4
    m4/pcre2_check_vscript.m4
    m4/pcre2_visibility.m4
    m4/pcre2_zos.m4
    pcre2-config.in
    src/libpcre2-*.sym.in
  ].freeze

  # C sources must contain only ASCII text. The release script also detrails
  # them, but performs the stricter encoding validation separately.
  C_FILES = %w[
    src/config-cmake.h.in
    src/config-meson.h.in
    src/pcre2.h.in
    src/pcre2_auto_possess.c
    src/pcre2_chartables.c.dist
    src/pcre2_chartables.c.ebcdic-1047-nl15
    src/pcre2_chartables.c.ebcdic-1047-nl25
    src/pcre2_chkdint.c
    src/pcre2_compile.c
    src/pcre2_compile.h
    src/pcre2_compile_cgroup.c
    src/pcre2_compile_class.c
    src/pcre2_config.c
    src/pcre2_context.c
    src/pcre2_convert.c
    src/pcre2_dfa_match.c
    src/pcre2_dftables.c
    src/pcre2_error.c
    src/pcre2_extuni.c
    src/pcre2_find_bracket.c
    src/pcre2_fuzzsupport.c
    src/pcre2_internal.h
    src/pcre2_intmodedep.h
    src/pcre2_jit_char_inc.h
    src/pcre2_jit_compile.c
    src/pcre2_jit_match_inc.h
    src/pcre2_jit_misc_inc.h
    src/pcre2_jit_simd_inc.h
    src/pcre2_jit_test.c
    src/pcre2_maketables.c
    src/pcre2_match.c
    src/pcre2_match_data.c
    src/pcre2_match_next.c
    src/pcre2_newline.c
    src/pcre2_ord2utf.c
    src/pcre2_pattern_info.c
    src/pcre2_printint_inc.h
    src/pcre2_script_run.c
    src/pcre2_serialize.c
    src/pcre2_string_utils.c
    src/pcre2_study.c
    src/pcre2_substitute.c
    src/pcre2_substring.c
    src/pcre2_tables.c
    src/pcre2_ucd.c
    src/pcre2_ucp.h
    src/pcre2_ucptables_inc.h
    src/pcre2_util.h
    src/pcre2_valid_utf.c
    src/pcre2_xclass.c
    src/pcre2demo.c
    src/pcre2grep.c
    src/pcre2posix.c
    src/pcre2posix.h
    src/pcre2posix_test.c
    src/pcre2test.c
    src/pcre2test_inc.h
  ].freeze

  # Keep the explicit list in sync with the C sources that need ASCII checks.
  source_root = "#{Pcre2Maintenance::ROOT}/"
  c_sources = (Dir.glob("#{source_root}src/*.{c,h}") +
    Dir.glob("#{source_root}src/*.{c,h}.in") +
    Dir.glob("#{source_root}src/pcre2_chartables.c.*")).map { |path| path.delete_prefix(source_root) }
  raise 'C_FILES does not match the C source files' unless C_FILES.sort == c_sources.sort

  module_function

  # Clean nroff -man output for online reading. Current nroff no longer emits
  # the repeated headers, footer chunks, or screen controls handled by the old
  # Perl script. It still uses overstriking for bold text.
  def nroff_output_to_plain_text(text)
    text.gsub(/.\x08/, '')
  end

  # Render a generated manpage to cleaned plain text.
  def render_man_to_text(manpage)
    nroff_output_to_plain_text(capture!('nroff', '-c', '-t', '-Kutf8', '-Tutf8',
      "-rLL=#{MANPAGE_WIDTH}n", '-man', "man/#{manpage}"))
  end

  # Remove trailing whitespace while preserving unterminated final lines.
  def detrail(filename)
    content = File.binread(filename)
    updated = content.lines.map { |line| line.sub(/[ \t\r\f\v]+\n\z/, "\n") }.join
    # Do not rewrite an unchanged file; release checks rely on idempotence.
    File.binwrite(filename, updated) unless updated == content
  end

  # Validate UTF-8, line endings, and optionally the ASCII-only C sources.
  def validate_text(filename, ascii: false, crlf: false)
    content = File.binread(filename).force_encoding(Encoding::UTF_8)
    unless content.valid_encoding?
      warn "Bad UTF-8 in #{filename}"
      return false
    end
    valid = true
    if !crlf && content.include?("\r")
      warn "CR in #{filename}"
      valid = false
    end
    if ascii && content.match?(/[^\x01-\x7e]/)
      warn "Non-ASCII in #{filename}"
      valid = false
    end
    valid
  end

  # Expand required glob patterns, rejecting a stale pattern that matches nothing.
  def expand_paths(patterns)
    patterns.flat_map do |pattern|
      paths = Dir.glob(pattern)
      # A missing glob indicates a stale maintenance file list, not an empty set.
      raise "No files found for #{pattern}" if paths.empty?
      paths
    end
  end

  # Verify that man can process every generated page without warnings.
  def validate_manpages
    puts 'Validating man pages'
    raise 'man-db does not support warnings' unless capture!('man', '--help').include?('warnings')

    Dir.glob('man/*.{1,3}').each do |file|
      _, warnings, status = Open3.capture3(
        {'MANROFFSEQ' => '', 'MANWIDTH' => MANPAGE_WIDTH.to_s},
        'man', '--warnings=w,all', '-E', 'UTF-8', '-l', '-Tutf8', '-Z', file
      )
      raise "Running man failed for #{file}:\n#{warnings}" unless status.success?
      raise "Running man generated warnings for #{file}:\n#{warnings}" unless warnings.empty?
    end
  end

  # Generate all derived documentation files for the current release.
  def update_documentation
    puts 'Processing documentation'
    Dir.chdir('doc') do
      # Generate the two AsciiDoc sources that are derived from project data.
      puts 'Making pcre2demo.adoc'
      Pcre2AsciidocDemo.run
      puts 'Making index.adoc'
      Pcre2AsciidocIndex.run

      # The AsciiDoc sources are canonical. Check their release version before
      # generating the retained troff and HTML output from them.
      sources = Dir.glob('*.adoc')
      sources.each do |file|
        next if file == 'index.adoc'
        unless File.read(file).lines.any? { |line| line.chomp == ":mansource: PCRE2 #{Pcre2Maintenance::CURRENT_RELEASE}" }
          raise "Version number in #{file} does not match current release"
        end
      end

      puts 'Making HTML and manpage documentation'
      FileUtils.mkdir_p(%w[html man])
      FileUtils.rm(Dir.glob('{html,man}/*'))
      FileUtils.cp('../README', 'html/README.txt')
      FileUtils.cp('../NON-AUTOTOOLS-BUILD', 'html/NON-AUTOTOOLS-BUILD.txt')

      sources.each do |file|
        if file == 'index.adoc'
          puts '  Making index.html'
          run!('asciidoctor', '--failure-level', 'WARN', '-v', '--require',
            '../maint/AsciidocConverterPcre2.rb', '--backend', 'html5',
            '--out-file', 'html/index.html', file)
          next
        end
        section = File.read(file)[/^:manvolnum: (.+)$/, 1]
        raise "Invalid manpage section in #{file}" unless %w[1 3].include?(section)
        base = File.basename(file, '.adoc')
        puts "  Making #{base}.#{section} and #{base}.html"
        run!('../maint/AsciidocRender', file, "man/#{base}.#{section}", "html/#{base}.html")
      end

      # Make tidy plain-text manuals for online reading. Concatenate the main
      # section-three manuals but omit individual function pages and pcre2demo.
      puts 'Making pcre2.txt'
      generated_manuals = Dir.glob('man/*.3').map { |path| File.basename(path, '.3') }
      excluded_manuals = generated_manuals.grep(/\Apcre2_/) + %w[pcre2demo]
      unless MANUALS.sort == (generated_manuals - excluded_manuals).sort
        raise 'MANUALS does not match the generated section-three manuals'
      end

      File.open('pcre2.txt', 'w:UTF-8') do |output|
        output << <<~TEXT
          -----------------------------------------------------------------------------
          This file contains a concatenation of the PCRE2 man pages, converted to plain
          text format for ease of searching with a text editor, or for use on systems
          that do not have a man page processor. The small individual files that give
          synopses of each function in the library have not been included. Neither has
          the pcre2demo program. There are separate text files for the pcre2grep and
          pcre2test commands.
          -----------------------------------------------------------------------------


        TEXT
        MANUALS.each do |base|
          puts "  Processing #{base}.3"
          output << render_man_to_text("#{base}.3")
          output << "#{'-' * 78}\n"
          output << "\n\n"
        end
      end
      # Render the three section-one command manuals as individual text files.
      %w[pcre2test pcre2grep pcre2-config].each do |base|
        puts "Making #{base}.txt"
        File.write("#{base}.txt", render_man_to_text("#{base}.1"), encoding: 'UTF-8')
      end

      # Validate the generated manpages after documentation rendering completes.
      validate_manpages

      puts 'Documentation done'
    end
  end

  # Update build.zig.zon to pin SLJIT to the checked-out dependency revision.
  def update_sljit_zig_dependency
    commit = capture!('git', 'rev-parse', 'HEAD:deps/sljit').strip
    raise 'deps/sljit has no commit' if commit.empty?

    if File.read('build.zig.zon').include?("sljit.git##{commit}")
      puts "SLJIT pinned in build.zig.zon to #{commit}"
    else
      puts "Updating SLJIT pin in build.zig.zon to #{commit}"
      run!('zig', 'fetch', '--save=sljit', "git+https://github.com/zherczeg/sljit.git##{commit}")
    end
  end

  # Regenerate release-maintained files, optionally limiting work to documentation.
  def update_always(arguments)
    raise ArgumentError, 'Usage: maint/UpdateAlways.rb [doc]' unless arguments.empty? || arguments == ['doc']

    # nroff must not replace ASCII hyphens with Unicode hyphens.
    ENV['LANG'] = ENV['LC_ALL'] = 'C'

    update_documentation
    return if arguments == ['doc']

    # Detrail selected source and generated documentation files, but never test
    # data, where trailing spaces may carry meaning.
    text_files = expand_paths(TEXT_FILES)
    c_files = expand_paths(C_FILES)
    puts 'Detrailing'
    (text_files + c_files).each { |file| detrail(file) }
    puts 'Validating all text'
    results = text_files.map { |file| validate_text(file) }
    results += c_files.map { |file| validate_text(file, ascii: true) }
    raise 'Text validation failed' unless results.all?

    update_sljit_zig_dependency

    # TODO validate build.zig.zon's version number too, and CMake/meson.
    # Generally check that UpdateRelease has been run!
    unless File.read('MODULE.bazel').include?("version = \"#{Pcre2Maintenance::CURRENT_RELEASE}\"")
      raise 'Version number in MODULE.bazel does not match current release'
    end

    puts 'Done'
  end
end

if $PROGRAM_NAME == __FILE__
  Dir.chdir(Pcre2Maintenance::ROOT) { Pcre2UpdateAlways.update_always(ARGV) }
end
