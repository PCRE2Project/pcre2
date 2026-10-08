# Keep the feature-test macro checks aligned with cmake/PCRE2UseSystemExtensions.cmake.
system_extension_args = []
if not host_is_windows
  getrlimit_test = '''
    #include <sys/time.h>
    #include <sys/resource.h>
    int main(void) {
      struct rlimit rlim;
      getrlimit(RLIMIT_STACK, &rlim);
      return 0;
    }
  '''
  if not cc.links(getrlimit_test, name: 'getrlimit without feature-test macros')
    if cc.links(getrlimit_test, args: '-D_ALL_SOURCE', name: 'getrlimit with _ALL_SOURCE')
      system_extension_args += '-D_ALL_SOURCE'
    endif
  endif
  mkostemp_test = '''
    #include <stdlib.h>
    int main(void) {
      char name[] = "pcre2-XXXXXX";
      return mkostemp(name, 0);
    }
  '''
  if not cc.links(mkostemp_test, werror: true, name: 'mkostemp without feature-test macros')
    if cc.links(mkostemp_test, args: '-D_GNU_SOURCE', werror: true, name: 'mkostemp with _GNU_SOURCE')
      system_extension_args += '-D_GNU_SOURCE'
    endif
  endif
endif
add_project_arguments(system_extension_args, language: 'c')
