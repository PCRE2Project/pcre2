#if defined(_WIN32)
#define FIXTURE_EXPORT __declspec(dllexport)
#else
#define FIXTURE_EXPORT __attribute__((visibility("default")))
#endif

FIXTURE_EXPORT int fixture_export(void)
{
return 0;
}