#ifndef CGFS_API_HPP
#define CGFS_API_HPP

#ifdef CGFS_STATIC
    #define CGFS_API
#elif defined(_WIN32) || defined(__CYGWIN__)
    #ifdef CGFS_BUILD_SHARED
        #define CGFS_API __declspec(dllexport)
    #elifdef CGFS_SHARED
        #define CGFS_API __declspec(dllimport)
    #else
        #error "Define CGFS_STATIC, CGFS_BUILD_SHARED or CGFS_SHARED."
    #endif
#elif defined(__GNUC__) || defined(__clang__)
    #define CGFS_API __attribute__((visibility("default")))
#else
    #define CGFS_API
#endif

#endif