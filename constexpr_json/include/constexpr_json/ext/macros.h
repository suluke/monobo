#ifndef CONSTEXPR_JSON_EXT_MACROS_H
#define CONSTEXPR_JSON_EXT_MACROS_H

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

#endif // CONSTEXPR_JSON_EXT_MACROS_H
