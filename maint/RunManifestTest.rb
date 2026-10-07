#! /usr/bin/env ruby
# frozen_string_literal: true

# Check an installation or unpacked distribution against its manifest.
# Each entry records its ls-style file type and permissions, a forward-slash
# path, and (for symlinks) the link target. The root directory is listed too.

require 'find'
require 'pathname'

def file_mode(stat)
  # Use the permission bits reported by lstat on every platform. On Windows,
  # Ruby derives them from the read-only attribute and filename extension:
  # ordinary writable files are 0644, directories and executables are 0755.
  # These synthesized bits are not a representation of Windows ACLs.
  type = {'file' => '-', 'directory' => 'd', 'link' => 'l', 'fifo' => 'p',
          'socket' => 's', 'characterSpecial' => 'c', 'blockSpecial' => 'b'}.fetch(stat.ftype)
  permissions = (0...9).map {|index| (stat.mode & (0400 >> index)) != 0 ? 'rwx'[index % 3] : '-' }.join
  # setuid/setgid/sticky replace the corresponding execute bit, just as in ls.
  [[04000, 2, 's', 'S'], [02000, 5, 's', 'S'], [01000, 8, 't', 'T']].each do |bit, index, on, off|
    permissions[index] = permissions[index] == 'x' ? on : off if (stat.mode & bit) != 0
  end
  type + permissions
end

def installed_files(input_dir, root_path)
  root = Pathname.new(File.realpath(input_dir))
  abort "#{input_dir}: not a directory" unless root.directory?
  entries = []
  # Never follow child symlinks or silently skip unreadable directories.
  Find.find(root.to_s, ignore_error: false) do |path|
    relative = Pathname.new(path).relative_path_from(root).to_s
    display = relative == '.' ? root_path : "#{root_path}/#{relative}"
    stat = File.lstat(path)
    line = "#{file_mode(stat)} #{display}"
    line += " -> #{File.readlink(path)}" if stat.symlink?
    entries << [display, line]
  end
  # Sort complete paths by UTF-8 bytes, like LC_ALL=C, not by locale or mode.
  # This also puts a directory before its contents. Paths containing spaces
  # need no special handling.
  entries.sort_by {|path, _line| path.encode('UTF-8').b }.map(&:last)
end

def expected_files(lines, producer, build_type)
  # Only transform the expected side. Unexpected installed files must remain
  # visible, even when they belong to a different build system.
  lines.each_with_object([]) do |line, result|
    if producer != 'tarball'
      if producer != 'cmake'
        next if line.match?(%r{/lib/cmake(?:/pcre2)?$})
        next if line.match?(%r{/lib/cmake/pcre2/pcre2-(?:config(?:-version)?|targets(?:-release)?)\.cmake$})
        line = line.sub(/\A-rw-r--r-- /, '-rwxr-xr-x ') if line.match?(%r{/libpcre2-(?:8|16|32|posix)\.so(?:\.\d+)+$})
      end
      if producer != 'autoconf'
        next if line.match?(%r{/lib/libpcre2-(?:8|16|32|posix)\.la$})
        if line.match?(%r{/libpcre2-(?:8|16|32|posix)\.so -> })
          line = line.sub(/(\.so\.\d+)\.\d+\.\d+$/, '\1')
        end
      end
      line = line.gsub('pcre2-targets-release.cmake', "pcre2-targets-#{build_type}.cmake")
    end
    result << line
  end
end

def main
  unless (3..4).cover?(ARGV.length)
    abort "Usage: ruby #{$PROGRAM_NAME} <dir> <manifest name> <producer> [<build type>]"
  end
  input_dir, manifest, producer, build_type = ARGV
  abort "Unknown producer: #{producer} (expected autoconf, cmake, tarball)" unless %w[autoconf cmake tarball].include?(producer)

  lines = File.readlines(manifest, encoding: 'UTF-8').map(&:chomp)
  abort "Empty manifest: #{manifest}" if lines.empty?
  # Anchor the listing at the manifest's root, regardless of the real install
  # prefix or DESTDIR. All manifests include this first directory entry.
  _mode, root_path = lines.first.split(' ', 2)
  abort "Missing root path in #{manifest}" unless root_path
  actual = installed_files(input_dir, root_path)
  expected = expected_files(lines, producer, build_type || 'release')

  base = File.basename(manifest)
  File.binwrite("#{base}.expected", expected.join("\n") + "\n")
  File.binwrite("#{base}.actual", actual.join("\n") + "\n")
  if expected != actual
    warn 'Missing or changed entries (-), unexpected entries (+):'
    (expected - actual).each {|line| warn "- #{line}" }
    (actual - expected).each {|line| warn "+ #{line}" }
    abort "Installed files differ from expected; compare #{base}.expected and #{base}.actual"
  end
  puts 'Installed files match expected'
  File.delete("#{base}.expected", "#{base}.actual")
end

main if $PROGRAM_NAME == __FILE__
