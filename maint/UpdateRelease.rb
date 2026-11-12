#! /usr/bin/env ruby
# frozen_string_literal: true

# Update all hardcoded release numbers in the source tree:
# - CMake and Meson build metadata;
# - AsciiDoc documentation sources;
# - the Bazel MODULE file and Zig package manifest.
#
# This script is run from the repository root.

require_relative 'UpdateCommon'

module Pcre2UpdateRelease
  extend Pcre2Maintenance

  CMAKE_VERSIONS = {
    'PCRE2_MAJOR' => 'pcre2_major',
    'PCRE2_MINOR' => 'pcre2_minor',
    'PCRE2_PRERELEASE' => 'pcre2_prerelease',
    'PCRE2_DATE' => 'pcre2_date',
    'LIBPCRE2_POSIX_VERSION' => 'libpcre2_posix_version',
    'LIBPCRE2_8_VERSION' => 'libpcre2_8_version',
    'LIBPCRE2_16_VERSION' => 'libpcre2_16_version',
    'LIBPCRE2_32_VERSION' => 'libpcre2_32_version'
  }.freeze

  # Update all release-version metadata from the current configure.ac values.
  def self.run
    info = Pcre2Maintenance::VERSION_INFO
    release = Pcre2Maintenance::CURRENT_RELEASE

    puts 'Updating CMakeLists.txt'
    CMAKE_VERSIONS.each do |variable, configure_name|
      update_file('CMakeLists.txt', /^set\(#{variable} ".*"\)$/,
        "set(#{variable} \"#{info.fetch(configure_name)}\")")
    end

    puts 'Updating meson.build'
    update_file('meson.build', /(\bproject\s*\(\s*'PCRE2'\s*,.*?\bversion\s*:\s*')[^']*(')/m) do |_, match|
      "#{match[1]}#{release}#{match[2]}"
    end
    update_file('meson.build', /(^pcre2_date[ \t]*=[ \t]*')[^']*(')/) do |_, match|
      "#{match[1]}#{info.fetch('pcre2_date')}#{match[2]}"
    end
    update_file('meson.build', /^libtool_versions[ \t]*=[ \t]*\{[^}]*\}/) do |content|
      %w[posix 8 16 32].each do |library|
        pattern = /('#{library}'\s*:\s*')[^']*(')/
        unless content.scan(pattern).length == 1
          raise "Expected exactly one #{library} entry in Meson libtool_versions"
        end
        content = content.gsub(pattern) do
          "#{Regexp.last_match(1)}#{info.fetch("libpcre2_#{library}_version")}#{Regexp.last_match(2)}"
        end
      end
      content
    end

    puts 'Updating AsciiDoc sources'
    Dir.glob('doc/*.adoc').each do |filename|
      next if filename == 'doc/index.adoc'
      puts "  Updating #{filename}"
      update_file(filename, /^:mansource: PCRE2 .*$/, ":mansource: PCRE2 #{release}")
    end

    # MODULE.bazel
    puts 'Updating MODULE.bazel'
    update_file('MODULE.bazel', /^    version = ".*?"/, "    version = \"#{release}\"")

    # Zig requires MAJOR.MINOR.PATCH[-prerelease], so insert .0 as the patch.
    puts 'Updating build.zig.zon'
    zig_version = release.sub(/\A(\d+\.\d+)(.*)\z/, '\1.0\2')
    update_file('build.zig.zon', /^    \.version = ".*?"/, "    .version = \"#{zig_version}\"")

  end
end

if $PROGRAM_NAME == __FILE__
  abort 'Usage: maint/UpdateRelease.rb' unless ARGV.empty?
  Dir.chdir(Pcre2Maintenance::ROOT) { Pcre2UpdateRelease.run }
end
