#! /usr/bin/env ruby
# frozen_string_literal: true

require 'open3'
require 'pathname'
require_relative 'UpdateCommon'

module Pcre2AsciidocDemo
  module_function

  # Generate the pcre2demo AsciiDoc page from its C source.
  def run
    repository = Pathname.new(__dir__).parent
    source_path = repository.join('src/pcre2demo.c')
    output_path = repository.join('doc/pcre2demo.adoc')

    date, status = Open3.capture2(
      Pcre2Maintenance::C_LOCALE,
      'git', '-C', repository.to_s, 'log', '-n1',
      '--date=format:%d %B %Y', '--format=%cd', '--', source_path.to_s
    )
    raise "Failed to read the date of #{source_path}" unless status.success?

    date = date.strip
    raise "No Git date found for #{source_path}" if date.empty?

    source = source_path.binread

    File.open(output_path, 'wb') do |output|
      output << <<~ADOC
        = pcre2demo(3)
        :doctype: manpage
        :manname: pcre2demo
        :manpurpose: a demonstration C program for PCRE2
        :description: A demonstration C program that uses the PCRE2 library
        :manvolnum: 3
        :manmanual: PCRE2
        :mansource: PCRE2 #{Pcre2Maintenance::CURRENT_RELEASE}
        :docdate: #{date}
        :revdate: #{date}

        == Source Code

        [source,c,role=wide]
        ----
      ADOC
      output << source
      output << "\n" unless source.end_with?("\n")
      output << "----\n"
    end
  end
end

if $PROGRAM_NAME == __FILE__
  Pcre2AsciidocDemo.run
end
