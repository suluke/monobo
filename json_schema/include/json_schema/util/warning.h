#ifndef JSON_SCHEMA_UTIL_WARNING_H
#define JSON_SCHEMA_UTIL_WARNING_H

#if defined(__GNUG__) && !defined(__clang__) && !defined(__INTEL_COMPILER)
#define MONOBO_PRAGMA_IMPL(UNQUOT) _Pragma(#UNQUOT)
#define MONOBO_PUSH_WARNING_MUTE_GCC(WARNING, REASON)                          \
  _Pragma("GCC diagnostic push") MONOBO_PRAGMA_IMPL(GCC diagnostic ignored WARNING)
#define MONOBO_POP_WARNING_GCC(WARNING) _Pragma("GCC diagnostic pop")

#else

#define MONOBO_PUSH_WARNING_MUTE_GCC(WARNING, REASON)
#define MONOBO_POP_WARNING_GCC(WARNING)

#endif

#endif // JSON_SCHEMA_UTIL_WARNING_H
