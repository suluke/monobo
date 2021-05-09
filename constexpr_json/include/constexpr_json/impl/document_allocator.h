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

  constexpr Entity allocateNumber(DocTy &theDoc, const double theNumber) {
    theDoc.itsNumbers[itsNumNumbers] = theNumber;
    return {Entity::NUMBER, itsNumNumbers++};
  }

  template <typename SourceEncodingTy, typename DestEncodingTy>
  constexpr typename ErrorHandlingTy::template ErrorOr<Entity>
  allocateTranscodeString(DocTy &theDoc, const std::string_view &theString,
                          const SourceEncodingTy &theSrcEnc,
                          const DestEncodingTy &theDestEnc) {
    std::string_view aStr = theString;
    using P = parsing<SourceEncodingTy>;
    const P p{theSrcEnc};
    size_t aNumBytesInStr = 0;
    theDoc.itsStrings[itsNumStrings].itsPosition = itsNumChars;
    while (aStr.size()) {
      const auto [aChar, aCharWidth] = theSrcEnc.decodeFirst(aStr);
      if (aCharWidth <= 0)
        return makeError("Failed to decode character");
      using CharT = typename SourceEncodingTy::CodePointTy;
      CharT aCodePoint = aChar;
      if (aChar == '\\') {
        const auto [aEscaped, aEscWidth] = p.parseEscape(aStr);
        aCodePoint = aEscaped;
        aStr.remove_prefix(aEscWidth);
      } else {
        aStr.remove_prefix(aCharWidth);
      }
      const auto [aBytes, aBytesUsed] = theDestEnc.encode(aCodePoint);
      if (aBytesUsed <= 0)
        return makeError("Failed to encode character");
      for (size_t i = 0; i < aBytesUsed; ++i)
        theDoc.itsChars[itsNumChars++] = aBytes[i];
      aNumBytesInStr += aBytesUsed;
    }
    theDoc.itsStrings[itsNumStrings].itsSize = aNumBytesInStr;
    return Entity{Entity::STRING, itsNumStrings++};
  }

  constexpr Entity allocateRawString(DocTy &theDoc,
                                     const std::string_view theString) {
    for (size_t aCharIdx = 0; aCharIdx < theString.size(); ++aCharIdx)
      theDoc.itsChars[itsNumChars + aCharIdx] = theString[aCharIdx];
    theDoc.itsStrings[itsNumStrings].itsPosition = itsNumChars;
    itsNumChars += theString.size();
    theDoc.itsStrings[itsNumStrings].itsSize = theString.size();
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
  template <typename JSON>
  constexpr Entity allocateJson(DocTy &theDoc, const JSON &theJson) {
    switch (theJson.getType()) {
    case Entity::ARRAY: {
      ptrdiff_t aIdx{0};
      const Entity aEntity = allocateArray(theDoc, theJson.toArray().size());
      const Array &aArray = theDoc.itsArrays[aEntity.itsPayload];
      for (const auto &aElement : theJson.toArray()) {
        theDoc.itsEntities[aArray.itsPosition + aIdx] =
            allocateJson(theDoc, aElement);
      }
      return aEntity;
    }
    case Entity::OBJECT: {
      ptrdiff_t aIdx{0};
      const Entity aEntity = allocateObject(theDoc, theJson.toObject().size());
      const Object &aObj = theDoc.itsObjects[aEntity.itsPayload];
      for (const auto &aKVPair : theJson.toObject()) {
        Property &aProp = theDoc.itsObjectProps[aObj.itsKeysPos + aIdx];
        aProp.itsKeyPos = allocateRawString(theDoc, aKVPair.first).itsPayload;
        theDoc.itsEntities[aObj.itsValuesPos + aIdx] =
            allocateJson(theDoc, aKVPair.second);
        ++aIdx;
      }
      return aEntity;
    }
    case Entity::NUL:
      return Entity{Entity::NUL, 0};
    case Entity::BOOL:
      return Entity{Entity::BOOL, theJson.toBool()};
    case Entity::NUMBER:
      return allocateNumber(theDoc, theJson.toNumber());
    case Entity::STRING:
      return allocateRawString(theDoc, theJson.toString());
    default:
      // gcc 7 actually raises a really funny error when we move this behind the
      // switch
      throw "Implementation error: Exhausted Json Type enum options";
    }
  }

  static constexpr auto makeError(const char *const theMsg,
                                  const ErrorCode theCode = ErrorCode::UNKNOWN)
      -> typename ErrorHandlingTy::template ErrorOr<Entity> {
    return ErrorHandlingTy::template makeError<Entity>(theCode, -1);
  }
};
} // namespace cjson
#endif // CONSTEXPR_JSON_DOCUMENT_ALLOCATOR_H
