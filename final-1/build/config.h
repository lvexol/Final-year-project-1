#define PACKAGE_NAME "vexovnc"
#define PACKAGE_VERSION "0.9"

/* #undef HAVE_ACTIVE_DESKTOP_H */
/* #undef HAVE_ACTIVE_DESKTOP_L */
/* #undef ENABLE_NLS */
#define HAVE_PAM

#define DATA_DIR "/home/vexo/project/Final-year-project-1/final-1/_install/share"
#define LOCALE_DIR "/home/vexo/project/Final-year-project-1/final-1/_install/share/locale"

/* MS Visual Studio 2008 and newer doesn't know ssize_t */
#if defined(HAVE_GNUTLS) && defined(WIN32) && !defined(__MINGW32__)
    #if defined(_WIN64)
        typedef __int64 ssize_t;
    #else
        typedef long ssize_t;
    #endif
#endif

/* We know we use deprecated stuff, it's to support older macOS */
#if defined(__APPLE__) && defined(__clang__)
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#endif
