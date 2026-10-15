#!/usr/bin/env ruby
# frozen_string_literal: true

ENV['BUNDLE_GEMFILE'] ||= File.expand_path('../pages/Gemfile', __dir__)
require 'bundler/setup'
require 'jekyll'
require 'optparse'
require 'uri'
require_relative 'UpdateCommon'
require_relative 'AsciidocConverterPcre2'

module Pcre2PagesBuild
  ROOT = File.expand_path('..', __dir__)

  module_function

  def build(source: File.join(ROOT, 'pages'), destination: nil, base_url: nil, doc_dir: nil, pagefind: true)
    base_url ||= ENV['PAGES_BASE_URL']
    if !base_url && ENV['CODESPACE_NAME'] && ENV['GITHUB_CODESPACES_PORT_FORWARDING_DOMAIN']
      base_url = "https://#{ENV.fetch('CODESPACE_NAME')}-1313.#{ENV.fetch('GITHUB_CODESPACES_PORT_FORWARDING_DOMAIN')}/"
    end
    raise ArgumentError, 'Specify --base-url or PAGES_BASE_URL' unless base_url

    uri = URI.parse(base_url)
    unless %w[http https].include?(uri.scheme) && uri.host && !uri.userinfo && !uri.query && !uri.fragment
      raise ArgumentError, "Invalid site base URL: #{base_url.inspect}"
    end
    source = File.expand_path(source)
    destination = File.expand_path(destination || File.join(source, 'public'))
    parent = destination.end_with?(File::SEPARATOR) ? destination : destination + File::SEPARATOR
    if [File::SEPARATOR, Dir.home, ROOT, source].include?(destination) || source.start_with?(parent)
      raise ArgumentError, 'The destination must not contain the source or repository root'
    end
    config = Jekyll.configuration(
      'source' => source, 'destination' => destination,
      'config' => File.join(source, '_config.yml'),
      'url' => "#{uri.scheme}://#{uri.host}#{uri.port == uri.default_port ? '' : ":#{uri.port}"}",
      'baseurl' => uri.path.sub(%r{/\z}, ''), 'release' => Pcre2Maintenance::CURRENT_RELEASE,
      'pcre2_root' => ROOT, 'pcre2_doc_dir' => File.expand_path(doc_dir || File.join(ROOT, 'doc'))
    )
    puts "Building site for release #{Pcre2Maintenance::CURRENT_RELEASE}"
    Jekyll::Site.new(config).process
    if pagefind
      raise 'Pagefind failed' unless system('npx', '-y', 'pagefind@1.5.2', '--site', destination)
    end
    destination
  end
end

if $PROGRAM_NAME == __FILE__
  options = {}
  parser = OptionParser.new do |cli|
    cli.banner = "Usage: #{$PROGRAM_NAME} [options]"
    cli.on('--source DIR', 'Jekyll source directory') { |value| options[:source] = value }
    cli.on('--destination DIR', 'Output directory') { |value| options[:destination] = value }
    cli.on('--base-url URL', 'Deployment URL, including any path prefix') { |value| options[:base_url] = value }
    cli.on('--doc-dir DIR', 'AsciiDoc source directory') { |value| options[:doc_dir] = value }
    cli.on('--no-pagefind', 'Build HTML without the search index') { options[:pagefind] = false }
    cli.on('-h', '--help', 'Show usage') { puts cli; exit }
  end
  parser.parse!
  raise OptionParser::InvalidArgument, ARGV.join(' ') unless ARGV.empty?
  Pcre2PagesBuild.build(**options)
end
