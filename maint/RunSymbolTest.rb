#! /usr/bin/env ruby
# frozen_string_literal: true

# Compare the four libraries' public exports with the canonical ELF symbol
# manifests. Native inspection tools supply the platform-specific records;
# command failures are fatal, so partial output cannot pass a comparison.

require 'open3'

def command_output(*command)
  output, status = Open3.capture2({'LANG' => 'C', 'LC_ALL' => 'C'}, *command)
  abort "Command failed (#{status}): #{command.join(' ')}" unless status.success?
  output
end

def find_dumpbin
  # An activated MSVC environment may already provide dumpbin. Otherwise use
  # Visual Studio's locator; building with CMake does not activate that shell.
  ENV.fetch('PATH', '').split(File::PATH_SEPARATOR).each do |directory|
    candidate = File.join(directory, 'dumpbin.exe')
    return candidate if File.file?(candidate)
  end
  vswhere = File.join(ENV.fetch('ProgramFiles(x86)', 'C:/Program Files (x86)'),
                     'Microsoft Visual Studio/Installer/vswhere.exe')
  paths = command_output(vswhere, '-utf8', '-latest', '-requires',
                         'Microsoft.VisualStudio.Component.VC.Tools.x86.x64',
                         '-find', 'VC\Tools\MSVC\*\bin\Hostx64\x64\dumpbin.exe')
  abort 'vswhere did not find dumpbin.exe' if paths.strip.empty?
  paths.lines.first.strip
end

def solaris_symbols(versions_output, symbols_output)
  # Solaris nm omits symbol versions. elfdump supplies a version-name table
  # and dynamic-symbol rows: index/value/size/type/bind/oth/ver/shndx/name.
  versions = {}
  versions_output.each_line do |line|
    match = line.match(/^\s*\[(\d+)\]\s+(\S+)/)
    versions[match[1]] = match[2] if match
  end
  symbols = []
  symbols_output.each_line do |line|
    next unless line.match?(/^\s*\[\d+\]/)
    fields = line.split
    abort "Unrecognized elfdump symbol: #{line}" if fields.length < 8
    version, section, name = fields.values_at(6, 7, 8)
    next if section == 'UNDEF' # Includes the unnamed null symbol.
    abort "Missing name in elfdump symbol: #{line}" unless name
    type = {'.text' => 'T', 'ABS' => 'A'}.fetch(section, section)
    # Keep unmapped version indexes visible: unversioned exports must not
    # accidentally compare equal to the versioned public API.
    symbols << "#{type} #{name}@@#{versions.fetch(version, version)}"
  end
  symbols
end

def nm_symbols(output)
  # Addresses vary with the build. Keep only the symbol type and name,
  # including any ELF version suffix, for comparison with the public API.
  output.lines.map {|line| line.strip.sub(/\A[0-9a-fA-F]+\s+/, '') }.reject(&:empty?)
end

def dll_symbols(output)
  # dumpbin's export rows begin with ordinal, hint and RVA; ignore its headings
  # and section summary, preserving the actual exported name verbatim.
  output.lines.each_with_object([]) do |line, symbols|
    match = line.match(/^\s*\d+\s+[0-9a-fA-F]+\s+[0-9a-fA-F]+\s+(\S+)/)
    symbols << "T #{match[1]}" if match
  end
end

def public_symbols(symbols, darwin)
  symbols.each_with_object([]) do |line, result|
    type, name = line.split(' ', 2)
    abort "Unrecognized symbol record: #{line}" unless type && name
    # Imports and unresolved weak references are not exports. Absolute PCRE2_
    # symbols name ELF version definitions rather than public functions/data.
    next if %w[U w].include?(type)
    next if type == 'A' && name.start_with?('PCRE2_')
    # These are linker-generated ELF bookkeeping symbols, not PCRE2 exports.
    next if %w[_init _fini __bss_start _end _DYNAMIC _GLOBAL_OFFSET_TABLE_
               _PROCEDURE_LINKAGE_TABLE_ _edata _etext].include?(name.split('@', 2).first)
    name = name.delete_prefix('_') if darwin
    result << "#{type} #{name}"
  end.sort_by(&:b)
end

def main
  abort "Usage: ruby #{$PROGRAM_NAME} <library dir> <manifest dir>" unless ARGV.length == 2
  input_dir, manifest_dir = ARGV
  windows = RUBY_PLATFORM.match?(/mswin|mingw/)
  darwin = RUBY_PLATFORM.include?('darwin')
  solaris = RUBY_PLATFORM.match?(/solaris|sunos/)
  dumpbin = find_dumpbin if windows
  extension = windows ? 'dll' : (darwin ? 'dylib' : 'so')

  %w[8 16 32 posix].each do |width|
    base = "manifest-libpcre2-#{width}.so"
    library = File.join(input_dir, "#{windows ? '' : 'lib'}pcre2-#{width}.#{extension}")
    abort "Library does not exist: #{library}" unless File.file?(library)
    if windows
      symbols = dll_symbols(command_output(dumpbin, '/exports', library))
    elsif solaris
      symbols = solaris_symbols(command_output('elfdump', '-v', library),
                                command_output('elfdump', '-T', 'SHT_DYNSYM', library))
    else
      tool = RUBY_PLATFORM.include?('freebsd') ? 'llvm-nm' : 'nm'
      arguments = ['-B', darwin ? '-g' : '-D']
      arguments << '--with-symbol-versions' if RUBY_PLATFORM.include?('linux')
      symbols = nm_symbols(command_output(tool, *arguments, library))
    end
    actual = public_symbols(symbols, darwin)
    expected = File.readlines(File.join(manifest_dir, base), encoding: 'UTF-8').map(&:chomp)
    abort "Empty symbol manifest: #{base}" if expected.empty?
    # Mach-O and PE do not carry the ELF symbol-version suffixes.
    expected = expected.map {|line| line.sub(/@.*$/, '') } if windows || darwin

    File.binwrite("#{base}.expected", expected.join("\n") + "\n")
    File.binwrite("#{base}.actual", actual.join("\n") + "\n")
    if expected != actual
      warn 'Missing or changed exports (-), unexpected exports (+):'
      (expected - actual).each {|line| warn "- #{line}" }
      (actual - expected).each {|line| warn "+ #{line}" }
      abort "Symbols for #{library} differ; compare #{base}.expected and #{base}.actual"
    end
    puts "Shared object contents for #{library} match expected"
    File.delete("#{base}.expected", "#{base}.actual")
  end
end

main if $PROGRAM_NAME == __FILE__
