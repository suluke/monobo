/// Mandatory Parameters:
#if !defined(NAME)
#error schema_loading_helper expects argument NAME ()
#endif
#if !defined(HEADER)
#error schema_loading_helper expects argument HEADER (JSON input file)
#endif

/// Optional Parameters:
#ifndef JSON_SCHEMA_STD
#define JSON_SCHEMA_STD Standard_2019_09
#endif
/// Optional Output Parameters:
#ifndef JSONNAME
#define JSONNAME(theName) a##theName##Json
#endif
#ifndef DOCNAME
#define DOCNAME(theName) a##theName##Doc
#endif
#ifndef INFONAME
#define INFONAME(theName) theName
#endif

/// Implementation:
#define _JSONNAME(theName) JSONNAME(theName)
#define _DOCNAME(theName) DOCNAME(theName)
#define _INFONAME(theName) INFONAME(theName)
#define STRINGIFY(Arg) _STRINGIFY(Arg)
#define _STRINGIFY(Arg) #Arg

// 1. Put the json into a string_view
#define USE_JSON_STRING(STR)                                                   \
  constexpr auto _JSONNAME(NAME) = std::string_view{STR};
#include HEADER

// 2. Load the string_view into a JSON
#include "constexpr_json/ext/macros.h"
LOAD_STATIC_JSON(_DOCNAME(NAME), _JSONNAME(NAME),
                 STRINGIFY(NAME) " schema JSON definition");

// 3. Compute the SchemaInfos required for static loading
// This is all this helper header does to allow for statically resolvable
// schema hyperlinks (multiple schemas loaded into same schema context)
constexpr auto _INFONAME(NAME) = JSON_SCHEMA_STD::SchemaInfoReader<
    std::decay_t<decltype(_DOCNAME(NAME)->getRoot())>,
    cjson::ErrorWillReturnNone>::read(_DOCNAME(NAME)->getRoot());

// 4. Assure successful loading
static_assert(_INFONAME(NAME),
              STRINGIFY(NAME) " SchemaInfo computation unsuccessful");

/// Cleanup
#undef _INFONAME
#undef INFONAME
#undef _JSONNAME
#undef JSONNAME
#undef _DOCNAME
#undef DOCNAME
#undef _STRINGIFY
#undef STRINGIFY

// FIXME I don't think anyone would want this more then once in their TU.
// But OTOH this leaks implementation details...
// #undef _LOAD_STATIC_JSON
// #undef LOAD_STATIC_JSON
// #undef CONSTEXPR_JSON_EXT_MACROS_H

// TODO Would be really cool to keep track of PRELOADED_SCHEMA_INFOS to offer
// a final macro putting it all together. Unfortunately, preprocessors only
// offer non-standard support for appending to existing #defines
// (see https://stackoverflow.com/a/45761452/1468532)

#undef HEADER
#undef JSON_SCHEMA_STD
#undef NAME
