#! /usr/bin/env ruby
# frozen_string_literal: true

# Check an installation or unpacked distribution against its manifest.
# Each entry records its ls-style file type and permissions, a forward-slash
# path, and (for symlinks) the link target. The root directory is listed too.

require 'find'
require 'pathname'
require_relative 'UpdateCommon'

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

def resolve_unversioned_targets(line, target)
  line.gsub(
    /@LIB_(?:8|16|32|POSIX)_UNVERSIONED_TARGET@/
  ) {|placeholder| placeholder.sub('UNVERSIONED_TARGET', target) }
end

def expand_library_versions(line, library_versions)
  line.gsub(/@LIB_(?<library>8|16|32|POSIX)_(?<kind>VERSION|SONAME)@/) do |placeholder|
    library = Regexp.last_match(:library)
    kind = Regexp.last_match(:kind)
    version = library_versions.fetch(library) do
      raise "Missing #{library} #{kind} value required by #{placeholder}"
    end
    kind == 'VERSION' ? version.version : version.soname
  end
end

def expected_files(lines, producer, build_type, library_versions)
  # An unpacked distribution has its own exact manifest, not an install layout.
  if producer == 'tarball'
    return lines
  end

  # The common install manifests include both CMake and libtool metadata. Adjust
  # only the expected listing: unexpected installed files must still fail the
  # check, even if another build system would legitimately install them.
  lines.each_with_object([]) do |line, result|
    if producer != 'cmake'
      # Only CMake installs find_package(CONFIG) metadata and exported targets.
      # Autoconf and Meson provide pkg-config metadata, but not these CMake files.
      #
      # NOTE: This CMake style is actually rather frustrating, because it exposes
      # CMake-specific metadata into the package's public installation layout.
      # Linux distributions probably should *not* distribute these files as a
      # result. The pkg-config metadata is more neutral.
      #
      # Exclude these CMake-specific files from the expected listing.
      next if line.match?(%r{/lib/cmake(?:/pcre2)?$})
      next if line.match?(%r{/lib/cmake/pcre2/pcre2-(?:config(?:-version)?|targets(?:-@CMAKE_BUILD@)?)\.cmake$})

      # The manifests use CMake's native ELF shared-library modes, which can be
      # 0644. Autoconf and Meson install the real .so files as 0755 instead.
      # Accept these native defaults; only real files, not symlinks, are adjusted.
      #
      # NOTE: There's a story here about why ELF permissions differ between
      # systems. Debian insists that shared libraries should not be executable,
      # whereas Fedora/RPM required them to be executable. So, there is no one
      # right answer on Linux, and the different build systems land in different
      # camps. For now, we are simply documenting and accepting whatever CMake,
      # Autoconf, and Meson do on Ubuntu (our CI base system).
      line = line.sub(/\A-rw-r--r-- /, '-rwxr-xr-x ') if line.match?(%r{/libpcre2-(?:8|16|32|posix)\.so\.@LIB_(?:8|16|32|POSIX)_VERSION@$})
    end

    if producer != 'autoconf'
      # Only Autoconf's libtool build installs .la archives containing link metadata.
      next if line.match?(%r{/lib/libpcre2-(?:8|16|32|posix)\.la$})
    end

    if producer == 'meson'
      # On macOS, Autoconf and CMake install a full-version dylib plus major-version
      # and unversioned symlinks. Meson's major-version file is real, so remove the
      # major-version symlink entry and rename the full-version file entry.
      # The unversioned link stays; current/compatibility versions remain in the
      # Mach-O metadata rather than requiring a full-version filename.
      #
      # CMake and Autoconf:
      #   -rwxr-xr-x /prefix/lib/libpcre2-8.VERSION.dylib
      #   lrwxr-xr-x /prefix/lib/libpcre2-8.SONAME.dylib -> libpcre2-8.VERSION.dylib
      #   lrwxr-xr-x /prefix/lib/libpcre2-8.dylib -> libpcre2-8.SONAME.dylib
      #
      # Meson:
      #   -rwxr-xr-x /prefix/lib/libpcre2-8.SONAME.dylib
      #   lrwxr-xr-x /prefix/lib/libpcre2-8.dylib -> libpcre2-8.SONAME.dylib
      #
      # Paper over this by storing the CMake/Autoconf convention in the manifest
      # on disc and rewriting it in the Meson case.
      next if line.match?(%r{\Al[^ ]* .*/libpcre2-(?:8|16|32|posix)\.@LIB_(?:8|16|32|POSIX)_SONAME@\.dylib -> })
      if line.match?(%r{\A-[^ ]* .*/libpcre2-(?:8|16|32|posix)\.@LIB_(?:8|16|32|POSIX)_VERSION@\.dylib$})
        line = line.sub(/\.@LIB_(8|16|32|POSIX)_VERSION@\.dylib$/, '.@LIB_\1_SONAME@.dylib')
      end
    end

    # CMake's configuration name is lower-case in its exported target filename.
    line = line.gsub('@CMAKE_BUILD@', build_type)

    # There's a difference in the symlink structures used by Autoconf and CMake/Meson.
    # We model this by using an UNVERSIONED_TARGET placeholder in the manifest, and
    # replace it by either SONAME or VERSION depending on whether the unversioned
    # library symlink points at the SONAME or full-VERSION file.
    #
    # Autoconf:
    #   lrwxrwxrwx /prefix/lib/libpcre2-8.so -> libpcre2-8.so.VERSION
    #   lrwxrwxrwx /prefix/lib/libpcre2-8.so.SONAME -> libpcre2-8.so.VERSION
    #   -rwxr-xr-x /prefix/lib/libpcre2-8.so.VERSION
    #
    # CMake/Meson:
    #   lrwxrwxrwx /prefix/lib/libpcre2-8.so -> libpcre2-8.so.SONAME
    #   lrwxrwxrwx /prefix/lib/libpcre2-8.so.SONAME -> libpcre2-8.so.VERSION
    #   -rwxr-xr-x /prefix/lib/libpcre2-8.so.VERSION
    line = resolve_unversioned_targets(line, producer == 'autoconf' ? 'VERSION' : 'SONAME')

    # Finally, replace all the version placeholders with the expected version numbers.
    line = expand_library_versions(line, library_versions)

    result << line
  end
end

def main
  unless (3..4).cover?(ARGV.length)
    abort "Usage: ruby #{$PROGRAM_NAME} <dir> <manifest name> <producer> [<build type>]"
  end
  input_dir, manifest, producer, build_type = ARGV
  abort "Unknown producer: #{producer} (expected autoconf, cmake, meson, or tarball)" unless %w[autoconf cmake meson tarball].include?(producer)

  lines = File.readlines(manifest, encoding: 'UTF-8').map(&:chomp)
  abort "Empty manifest: #{manifest}" if lines.empty?
  # Anchor the listing at the manifest's root, regardless of the real install
  # prefix or DESTDIR. All manifests include this first directory entry.
  # For example, an Autoconf install staged at install-dir/usr/local must have
  # the same manifest paths as a CMake or Meson install at install-dir.
  _mode, root_path = lines.first.split(' ', 2)
  abort "Missing root path in #{manifest}" unless root_path
  actual = installed_files(input_dir, root_path)
  versions = Pcre2Maintenance.library_filename_versions
  expected = expected_files(lines, producer, build_type || 'release', versions)

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
