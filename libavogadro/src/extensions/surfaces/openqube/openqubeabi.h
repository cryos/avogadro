#ifndef OPENQUBEABI_H
#define OPENQUBEABI_H

// If we are using a recent GCC version with visibility support use it
// The Krazy checker doesn't like this, but it's a valid CMake conditional
// krazy:excludeall=cpp
#if __GNUC__ >= 4
  #define OQ_ABI_IMPORT __attribute__ ((visibility("default")))
  #define OQ_ABI_EXPORT __attribute__ ((visibility("default")))
  #define OQ_ABI_HIDDEN __attribute__ ((visibility("hidden")))
#elif defined(_WIN32) || defined(__CYGWIN__)
  #define OQ_ABI_IMPORT __declspec(dllimport)
  #define OQ_ABI_EXPORT __declspec(dllexport)
  #define OQ_ABI_HIDDEN
#else
  #define OQ_ABI_IMPORT
  #define OQ_ABI_EXPORT
  #define OQ_ABI_HIDDEN
#endif

// This macro should be used to export parts of the API
#ifndef OPENQUBE_EXPORT
  #ifdef openqube_EXPORTS
    #define OPENQUBE_EXPORT OQ_ABI_EXPORT
  #else
    #define OPENQUBE_EXPORT OQ_ABI_IMPORT
  #endif
#endif

#endif // OPENQUBEABI_H
