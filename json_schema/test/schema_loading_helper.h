#if !defined(NAME) || !defined(HEADER)
#error schema_loading_helper expects arguments NAME and HEADER as definitions
#endif

#define LOAD_STATIC_JSON(theOutput, theJsonStr, theDesc)                       \
  _LOAD_STATIC_JSON(theOutput, theJsonStr, theDesc)
#define _LOAD_STATIC_JSON(theOutput, theJsonStr, theDesc)                      \
  constexpr auto aDocInfo##theOutput =                                         \
      cjson::DocumentParser<>::computeDocInfo(theJsonStr);                     \
  if (!aDocInfo##theOutput)                                                    \
    return 1;                                                                  \
  using DocTy##theOutput = CJSON_STATIC_DOCTY(*aDocInfo##theOutput);           \
  constexpr auto theOutput =                                                   \
      cjson::DocumentParser<>::parseDocument<DocTy##theOutput>(                \
          theJsonStr, *aDocInfo##theOutput);                                   \
  static_assert(theOutput, "Failed to parse " theDesc)

#define STRINGIFY(Arg) _STRINGIFY(Arg)
#define _STRINGIFY(Arg) #Arg

#define DOCNAME(theName) _DOCNAME(theName)
#define _DOCNAME(theName) a##theName##Doc

#define JSONNAME(theName) _JSONNAME(theName)
#define _JSONNAME(theName) a##theName##Json

#define INFONAME(theName) _INFONAME(theName)
#define _INFONAME(theName) a##theName##Info

#define USE_JSON_STRING(STR)                                                   \
  constexpr auto JSONNAME(NAME) = std::string_view{STR};
#include HEADER
LOAD_STATIC_JSON(DOCNAME(NAME), JSONNAME(NAME),
                 STRINGIFY(NAME) " schema JSON definition");
constexpr auto INFONAME(NAME) =
    SchemaInfoReader<std::decay_t<decltype(DOCNAME(NAME)->getRoot())>,
                     cjson::ErrorWillReturnNone>::read(DOCNAME(NAME)
                                                           ->getRoot());
static_assert(INFONAME(NAME),
              STRINGIFY(NAME) " SchemaInfo computation unsuccessful");

#undef _INFONAME
#undef INFONAME
#undef _JSONNAME
#undef JSONNAME
#undef _DOCNAME
#undef DOCNAME
#undef _STRINGIFY
#undef STRINGIFY
#undef _LOAD_STATIC_JSON
#undef LOAD_STATIC_JSON

#undef HEADER
#undef NAME
