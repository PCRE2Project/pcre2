# Keep the probes and their order aligned with cmake/PCRE2CheckVscript.cmake.
have_vscript = false
vscript_flag = ''
have_vscript_no_star = false
if not msvc_like
  vscript_source = '''
    int hidethis(void) { return 0; }
    int exposethis(void) { return hidethis(); }
  '''
  vscript_maps = {}
  foreach kind, contents : {
    '': 'PCRE2_10.00 { global: exposethis; local: *; };',
    '-broken': 'PCRE2_10.00 { global: exposethis; local: *; };  {',
    '-no-star': 'PCRE2_10.00 { global: exposethis; local: hidethis; };',
  }
    vscript_maps += {
      kind: configure_file(
        input: 'test-map-file.sym.in',
        output: 'test-map-file' + kind + '.sym',
        configuration: {'MAP_CONTENTS': contents},
      ),
    }
  endforeach

  # links() normally builds an executable. Probe a shared library instead, so
  # executable startup symbols cannot interfere with the map's local symbols.
  vscript_shared_args = cc.get_supported_arguments('-fPIC')
  vscript_shared_args += host_machine.system() == 'darwin' ? '-dynamiclib' : '-shared'
  foreach flag : ['--version-script', '-M']
    if cc.links(
      vscript_source,
      args: vscript_shared_args + ['-Wl,' + flag + ',' + vscript_maps[''].full_path()],
      name: 'shared library version script (' + flag + ')',
    )
      have_vscript = true
      vscript_flag = flag
      break
    endif
  endforeach

  if have_vscript
    if cc.links(
      vscript_source,
      args: vscript_shared_args + ['-Wl,' + vscript_flag + ',' + vscript_maps['-broken'].full_path()],
      name: 'broken version script',
    )
      have_vscript = false
      vscript_flag = ''
      message('Disabling version scripts: linker accepts invalid scripts.')
    endif
  endif

  if have_vscript
    have_vscript_no_star = cc.links(
      vscript_source,
      args: vscript_shared_args + ['-Wl,' + vscript_flag + ',' + vscript_maps['-no-star'].full_path()],
      name: 'version script without wildcard',
    )
  endif
endif
