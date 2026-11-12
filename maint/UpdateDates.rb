#! /usr/bin/env ruby
# frozen_string_literal: true

# Update all hardcoded documentation and README dates, ignoring generated-only
# commits. This script is run from the repository root.
#
# Documentation manpages have "Last updated" headers and footers, and so do
# the top-level READMEs. Generated sources such as pcre2demo and index do not
# track their own Git history and are deliberately skipped.

require_relative 'UpdateCommon'

module Pcre2UpdateDates
  extend Pcre2Maintenance

  DATE = /\d+ (?:Jan|Feb|Mar|Apr|May|Jun|Jul|Aug|Sep|Oct|Nov|Dec)\w* \d+/
  LAST_UPDATED = /^Last updated: #{DATE}/
  ADOC_DATE = /^:(docdate|revdate): #{DATE}$/

  # Return the latest non-generated Git commit date for a file.
  def self.last_date(filename)
    # Ignore commits marked #noupdate, which contain generated changes rather
    # than an edit to the source document itself.
    date = capture!(Pcre2Maintenance::C_LOCALE, 'git', 'log', '-n1', '--date=format:%d %B %Y', '--format=%cd',
      '--grep', '#noupdate', '--invert-grep', '--', filename).strip
    raise "No Git date found for #{filename}" if date.empty?
    date
  end

  # Update AsciiDoc, README.md, and NON-AUTOTOOLS-BUILD.md date metadata.
  def self.run
    puts 'Updating man pages'

    # doc/*.adoc
    Dir.glob('doc/*.adoc').each do |filename|
      next if %w[doc/pcre2demo.adoc doc/index.adoc].include?(filename)
      puts "  Updating #{filename}"
      date = last_date(filename)
      update_file(filename, ADOC_DATE) { |_, match| ":#{match[1]}: #{date}" }
      if filename.start_with?('doc/pcre2_')
        raise "Pattern unexpectedly found in #{filename}" if File.read(filename, encoding: Encoding::UTF_8).match?(LAST_UPDATED)
      else
        update_file(filename, LAST_UPDATED, "Last updated: #{date}")
      end
    end

    puts 'Updating README.md and NON-AUTOTOOLS-BUILD.md'
    %w[README.md NON-AUTOTOOLS-BUILD.md].each do |filename|
      update_file(filename, LAST_UPDATED, "Last updated: #{last_date(filename)}")
    end
  end
end

if $PROGRAM_NAME == __FILE__
  abort 'Usage: maint/UpdateDates.rb' unless ARGV.empty?
  Dir.chdir(Pcre2Maintenance::ROOT) { Pcre2UpdateDates.run }
end
