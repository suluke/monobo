#ifndef CONSTEXPR_JSON_DOCUMENT_BUILDER_H
#define CONSTEXPR_JSON_DOCUMENT_BUILDER_H

#include "document.h"
#include "utils/parsing.h"
#include "utils/unicode.h"
#include <memory>

namespace cjson {

struct DocumentInfo {
  size_t itsNumDoubles;
  size_t itsNumChars;
  size_t itsNumStrings;
  size_t itsNumArrays;
  size_t itsNumArrayEntries;
  size_t itsNumObjects;
  size_t itsNumObjectProperties;

  operator bool() const {
    return itsNumDoubles >= 0 && itsNumChars >= 0 && itsNumStrings >= 0 &&
           itsNumArrays >= 0 && itsNumArrayEntries >= 0 && itsNumObjects >= 0 &&
           itsNumObjectProperties >= 0;
  }
};

template <size_t N>
constexpr DocumentInfo computeDocInfo(StrLiteralRef<N> theJsonString) {
  return {};
}

template <size_t N, typename DocTy>
constexpr bool parseDocument(StrLiteralRef<N> theJsonString, DocTy &theDoc) {
  return true;
}

template <size_t N>
std::unique_ptr<DocumentBase> buildDocument(StrLiteralRef<N> theJsonString) {
  constexpr DocumentInfo aDocInfo = computeDocInfo(theJsonString);
  if (!aDocInfo)
    return nullptr;
  using DocTy = Document<aDocInfo.itsNumDoubles, aDocInfo.itsNumChars,
                         aDocInfo.itsNumStrings, aDocInfo.itsNumArrays,
                         aDocInfo.itsNumArrayEntries, aDocInfo.itsNumObjects,
                         aDocInfo.itsNumObjectProperties>;
  std::unique_ptr<DocTy> aDoc = std::make_unique<DocTy>();
  auto aParseResult = parseDocument(theJsonString, *aDoc);
  if (!aParseResult) {
    return nullptr;
  }
  return aDoc;
}

} // namespace cjson
#endif // CONSTEXPR_JSON_DOCUMENT_BUILDER_H
