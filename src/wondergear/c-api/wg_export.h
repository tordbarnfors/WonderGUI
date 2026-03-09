#ifndef WG_EXPORT_H
#define WG_EXPORT_H

#ifdef WG_STATIC_DEFINE
#  define WG_EXPORT
#  define WG_NO_EXPORT
#else
#  ifndef WG_EXPORT
#    ifdef wondergear_EXPORTS
        /* We are building this library */
#      define WG_EXPORT 
#    else
        /* We are using this library */
#      define WG_EXPORT 
#    endif
#  endif

#  ifndef WG_NO_EXPORT
#    define WG_NO_EXPORT 
#  endif
#endif

#ifndef WG_DEPRECATED
#  define WG_DEPRECATED __attribute__ ((__deprecated__))
#endif

#ifndef WG_DEPRECATED_EXPORT
#  define WG_DEPRECATED_EXPORT WG_EXPORT WG_DEPRECATED
#endif

#ifndef WG_DEPRECATED_NO_EXPORT
#  define WG_DEPRECATED_NO_EXPORT WG_NO_EXPORT WG_DEPRECATED
#endif

/* NOLINTNEXTLINE(readability-avoid-unconditional-preprocessor-if) */
#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef WG_NO_DEPRECATED
#    define WG_NO_DEPRECATED
#  endif
#endif

#endif /* WG_EXPORT_H */