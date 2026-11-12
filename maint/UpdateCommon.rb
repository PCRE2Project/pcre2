# frozen_string_literal: true

# Shared helpers for the maintenance update scripts.

require 'open3'

module Pcre2Maintenance
  LibraryFilenameVersion = Struct.new(:version, :soname, keyword_init: true)

  ROOT = File.expand_path('..', __dir__)
  C_LOCALE = {'LANG' => 'C', 'LC_ALL' => 'C'}.freeze

  # Extract release and ABI version definitions from a configure.ac file.
  def self.version_info
    entries = File.read(File.join(ROOT, 'configure.ac'), encoding: Encoding::UTF_8).scan(
      /m4_define\((pcre2_(?:major|minor|prerelease|date)|libpcre2_(?:posix|8|16|32)_version), *\[(.*?)\]\)/
    )
    raise 'Duplicate version definitions in configure.ac' unless entries.map(&:first).uniq.length == entries.length

    entries.to_h
  end

  private_class_method :version_info

  VERSION_INFO = version_info.freeze
  CURRENT_RELEASE = "#{VERSION_INFO.fetch('pcre2_major')}.#{VERSION_INFO.fetch('pcre2_minor')}#{VERSION_INFO.fetch('pcre2_prerelease')}"

  module_function

  # Return the filename version and SONAME for each library from its libtool
  # current:revision:age definition.
  def library_filename_versions
    VERSION_INFO.each_with_object({}) do |(definition, value), versions|
      match = /\Alibpcre2_(posix|8|16|32)_version\z/.match(definition)
      next unless match

      components = value.split(':', -1).map(&:strip)
      unless components.length == 3 && components.all? {|component| component.match?(/\A\d+\z/) }
        raise "Invalid libtool version for #{definition}: #{value.inspect}"
      end

      current, revision, age = components.map(&:to_i)
      raise "Invalid libtool version for #{definition}: current is less than age" if current < age

      library = match[1].upcase
      soname = current - age
      versions[library] = LibraryFilenameVersion.new(
        version: "#{soname}.#{age}.#{revision}",
        soname: soname.to_s
      ).freeze
    end.freeze
  end

  # Update every match in a UTF-8 file. A missing match is an error: it usually
  # means that the source format changed and the maintenance script needs work.
  # Rewrites the file only when the replacement changes its contents.
  def update_file(filename, pattern, replacement = nil, &block)
    content = File.read(filename, encoding: 'UTF-8')
    raise "Pattern not found in #{filename}: #{pattern.inspect}" unless content.match?(pattern)

    updated = if block
      content.gsub(pattern) { |text| block.call(text, Regexp.last_match) }
    else
      content.gsub(pattern, replacement)
    end
    File.write(filename, updated, encoding: 'UTF-8') unless updated == content
  end

  # Raise if a command cannot be run successfully, rather than allowing a
  # maintenance update to continue after a partial failure.
  # Returns true when the command exits successfully.
  def run!(*command)
    system(*command, exception: true)
  end

  # Run a command and return its standard output.
  def capture!(*command)
    output, status = Open3.capture2(*command)
    raise "Command failed (#{status.exitstatus}): #{command.inspect}" unless status.success?

    output
  end

end
