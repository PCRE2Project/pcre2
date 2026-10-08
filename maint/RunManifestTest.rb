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
  # The common install manifests include both CMake and libtool metadata. Adjust
  # only the expected listing: unexpected installed files must still fail the
  # check, even if another build system would legitimately install them.
  lines.each_with_object([]) do |line, result|
    # An unpacked distribution has its own exact manifest, not an install layout.
    if producer != 'tarball'
      if producer != 'cmake'
        # Only CMake installs find_package(CONFIG) metadata and exported targets.
        # Autoconf and Meson provide pkg-config metadata, but not these CMake files.
        #
        # NOTE: This CMake style is actually rather frustrating, because it exposes
        # CMake-specific metadata into the package's public installation layout.
        # Linux distributions probably should *not* distribute these files as a
        # result. The pkg-config metadata is more neutral.
        next if line.match?(%r{/lib/cmake(?:/pcre2)?$})
        next if line.match?(%r{/lib/cmake/pcre2/pcre2-(?:config(?:-version)?|targets(?:-release)?)\.cmake$})

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
        line = line.sub(/\A-rw-r--r-- /, '-rwxr-xr-x ') if line.match?(%r{/libpcre2-(?:8|16|32|posix)\.so(?:\.\d+)+$})
      end
      if producer != 'autoconf'
        # Only Autoconf's libtool build installs .la archives containing link metadata.
        next if line.match?(%r{/lib/libpcre2-(?:8|16|32|posix)\.la$})
        # The manifests use Autoconf's direct unversioned .so -> full-version file
        # link. CMake and Meson instead link via the SONAME (.so.N) symlink.
        # Both layouts resolve to the same library; retain each producer's convention.
        if line.match?(%r{/libpcre2-(?:8|16|32|posix)\.so -> })
          line = line.sub(/(\.so\.\d+)\.\d+\.\d+$/, '\1')
        end
      end
      if producer == 'meson'
        # On macOS, Autoconf and CMake install a full-version dylib plus major-version
        # and unversioned symlinks. Meson's major-version file is real, so remove the
        # major-version symlink entry and rename the full-version file entry.
        # The unversioned link stays; current/compatibility versions remain in the
        # Mach-O metadata rather than requiring a full-version filename.
        next if line.match?(%r{\Al[^ ]* .*/libpcre2-(?:8|16|32|posix)\.\d+\.dylib -> })
        if line.match?(%r{\A-[^ ]* .*/libpcre2-(?:8|16|32|posix)\.\d+\.\d+\.\d+\.dylib$})
          line = line.sub(/(\.\d+)\.\d+\.\d+\.dylib$/, '\1.dylib')
        end
      end
      # The manifests list CMake's Release export. Other configurations use the
      # lower-case build_type in the filename, for example relwithdebinfo.
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
