#ifndef CONSTEXPR_JSON_DOCUMENT_ALLOCATOR_H
#define CONSTEXPR_JSON_DOCUMENT_ALLOCATOR_H

#include "constexpr_json/document_info.h"
#include "constexpr_json/impl/document_access.h"
#include "constexpr_json/impl/parsing_utils.h"

namespace cjson {
template <typename DocTy, typename ErrorHandlingTy>
struct DocumentAllocator : private DocumentInfo {
  using EntityRef = typename DocTy::EntityRef;
  using ObjectRef = typename EntityRef::ObjectRef;
  using ArrayRef = typename EntityRef::ArrayRef;

  using DocumentInfo::itsNumObjectProperties;

  constexpr Entity allocateNumber(DocTy &theDoc, const double theNumber) {
    theDoc.itsNumbers[itsNumNumbers] = theNumber;
    return {Entity::NUMBER, itsNumNumbers++};
  }

  template <typename SourceEncodingTy, typename DestEncodingTy>
  constexpr typename ErrorHandlingTy::template ErrorOr<Entity>
  allocateTranscodeString(DocTy &theDoc, const std::string_view &theString) {
    std::string_view aStr = theString;
    using p = parsing<SourceEncodingTy>;
    size_t aNumBytesInStr = 0;
    theDoc.itsStrings[itsNumStrings].itsPosition = itsNumChars;
    while (aStr.size()) {
      const auto [aChar, aCharWidth] = SourceEncodingTy::decodeFirst(aStr);
      if (aCharWidth <= 0)
        return makeError("Failed to decode character");
      using CharT = typename SourceEncodingTy::CodePointTy;
      CharT aCodePoint = aChar;
      if (aChar == '\\') {
        const auto [aEscaped, aEscWidth] = p::parseEscape(aStr);
        aCodePoint = aEscaped;
        aStr.remove_prefix(aEscWidth);
      } else {
        aStr.remove_prefix(aCharWidth);
      }
      const auto [aBytes, aBytesUsed] = DestEncodingTy::encode(aCodePoint);
      if (aBytesUsed <= 0)
        return makeError("Failed to encode character");
      for (size_t i = 0; i < aBytesUsed; ++i)
        theDoc.itsChars[itsNumChars++] = aBytes[i];
      aNumBytesInStr += aBytesUsed;
    }
    theDoc.itsStrings[itsNumStrings].itsSize = aNumBytesInStr;
    return Entity{Entity::STRING, itsNumStrings++};
  }

  constexpr Entity allocateArray(DocTy &theDoc, size_t theSize) {
    auto &aArr = theDoc.itsArrays[itsNumArrays];
    aArr.itsPosition = 1 + itsNumArrayEntries + itsNumObjectProperties;
    aArr.itsNumElements = theSize;
    itsNumArrayEntries += theSize;
    return {Entity::ARRAY, itsNumArrays++};
  }
  constexpr Entity allocateObject(DocTy &theDoc, size_t theSize) {
    auto &aObj = theDoc.itsObjects[itsNumObjects];
    aObj.itsKeysPos = itsNumObjectProperties;
    aObj.itsValuesPos = 1 + itsNumArrayEntries + itsNumObjectProperties;
    aObj.itsNumProperties = theSize;
    itsNumObjectProperties += theSize;
    return Entity{Entity::OBJECT, itsNumObjects++};
  }
  // constexpr Object allocateObject(DocTy &theDoc, size_t theSize) {}

  static constexpr auto makeError(const char *const theMsg,
                                  const ErrorCode theCode = ErrorCode::UNKNOWN)
      -> typename ErrorHandlingTy::template ErrorOr<Entity> {
    return ErrorHandlingTy::template makeError<Entity>(theCode, -1);
  }
};
} // namespace cjson
#endif // CONSTEXPR_JSON_DOCUMENT_ALLOCATOR_H
