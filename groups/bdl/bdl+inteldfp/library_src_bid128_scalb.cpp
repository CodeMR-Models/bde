#include <bsls_platform.h>

#ifdef __GNUC__
#  define _WCHAR_T
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wcast-qual"
#elif !defined(BSLS_PLATFORM_OS_WINDOWS)
#  define __thread
#  define __QNX__
#endif

#ifndef BSLS_PLATFORM_OS_WINDOWS
#  define LINUX
#  define efi2
#else
#  define WINNT
#  define WINDOWS
#  define WNT
#  ifdef BSLS_PLATFORM_CPU_32_BIT
#    define ia32
#  else
#    define efi2
#  endif
#endif

#ifdef BSLS_PLATFORM_IS_BIG_ENDIAN
#define BID_BIG_ENDIAN 1
#else
#define BID_BIG_ENDIAN 0
#endif


extern "C" {
  #include "LIBRARY/src/bid128_scalb.c"
}

#ifdef __GNUC__
#  pragma GCC diagnostic pop
#endif
