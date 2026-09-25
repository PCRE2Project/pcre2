#!/usr/bin/env ruby
# frozen_string_literal: true

require 'pathname'

DOCUMENT_TITLE = 'Perl-compatible Regular Expressions (PCRE2 API)'
INTRODUCTION = <<~ADOC
  The HTML documentation for PCRE2 consists of a number of pages that are listed
  below in alphabetical order. If you are new to PCRE2, please read the first one
  first.
ADOC
API_INTRODUCTION = <<~ADOC
  There are also individual pages that summarize the interface for each function
  in the library.
ADOC

Page = Struct.new(:source, :name, :purpose, keyword_init: true)

def read_attribute(source, name, required: true)
  values = source.read(encoding: Encoding::UTF_8).lines.map do |line|
    match = line.chomp.match(/\A:#{Regexp.escape(name)}:\s+(.+)\z/)
    match[1] if match
  end.compact

  raise "#{source}: missing :#{name}: attribute" if required && values.empty?
  raise "#{source}: duplicate :#{name}: attribute" if values.length > 1

  values.first
end

def read_page(source)
  name = read_attribute(source, 'manname')
  expected_name = source.basename('.adoc').to_s
  raise "#{source}: :manname: #{name.inspect} does not match #{expected_name.inspect}" if name != expected_name

  purpose = read_attribute(source, 'description', required: false)
  purpose ||= read_attribute(source, 'manpurpose').sub(/\A./, &:upcase)
  Page.new(source:, name:, purpose:)
end

def table_cell(text)
  raise "table cell contains a newline: #{text.inspect}" if text.match?(/[\r\n]/)

  text.gsub('|', '\|')
end

def write_table(output, pages)
  output << "[cols=\"2,3\",stripes=even]\n"
  output << "|===\n"
  pages.each do |page|
    output << "|xref:#{page.source.basename}[#{table_cell(page.name)}]\n"
    output << "|#{table_cell(page.purpose)}\n\n"
  end
  output << "|===\n"
end

repository = Pathname.new(__dir__).parent
documentation = repository.join('doc')
output_path = documentation.join('index.adoc')

pages = documentation.glob('pcre2*.adoc').map {|source| read_page(source) }.sort_by(&:name)
manuals, api_pages = pages.partition {|page| !page.name.start_with?('pcre2_') }

File.open(output_path, 'w:UTF-8') do |output|
  output << "= #{DOCUMENT_TITLE}\n"
  output << ":doctype: article\n"
  output << ":title: PCRE2 documentation\n\n"
  output << INTRODUCTION
  output << "\n"
  write_table(output, manuals)
  output << "\n"
  output << API_INTRODUCTION
  output << "\n"
  write_table(output, api_pages)
end
