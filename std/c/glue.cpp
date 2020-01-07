#include <cstdio>
#ifdef _WIN32
#define DLLEXPORT __declspec(dllexport)
#else
#define DLLEXPORT
#endif

extern "C" DLLEXPORT void putChar(int c) {
  fputc(c, stderr);
}