#ifndef CONSTEXPR_JSON_DOCUMENT_BUILDER2_H
#define CONSTEXPR_JSON_DOCUMENT_BUILDER2_H

#include "constexpr_json/document.h"
#include "constexpr_json/document_info.h"
#include "constexpr_json/utils/parsing.h"

namespace cjson {
template <typename SourceEncodingTy, typename DestEncodingTy>
struct DocumentBuilder2 {
private:
  using p = parsing<SourceEncodingTy>;
  using ElementId = intptr_t;
  using ParentId = intptr_t;
  using Location = intptr_t;
  using Type = typename p::Type;
  struct ElementInfo {
    ElementId itsId = 0;
    Type itsType;
    ParentId itsParentId = -1;
    Location itsLocation = 0;
    ElementId itsFirstChild = -1;
    ElementId itsLastChild = -1;
    ElementId itsNextSibling = -1;
    size_t itsNumChildren = 0;
  };

public:
  constexpr static DocumentInfo
  computeDocInfo(const std::string_view theJsonString) {
    return DocumentInfo::compute<SourceEncodingTy, DestEncodingTy>(
               theJsonString)
        .first;
  }

  template <typename DocTy>
  static constexpr std::optional<DocTy>
  parseDocument(const std::string_view theJsonString,
                const DocumentInfo &theDocInfo) {
    if (!theDocInfo)
      return makeError<DocTy>("Using illegal DocInfo for parsing");
    const auto aElementInfos = computeElementInfos<DocTy>(theJsonString, theDocInfo);
    if (!aElementInfos)
      return makeError<DocTy>("Failed to compute element infos");
    const ElementInfo *aCurrentElm = &aElementInfos->front();
    intptr_t aNumEntitiesAlloced = 1;
    intptr_t aNumPropsAlloced = 0;
    intptr_t aNumNumbers = 0;
    intptr_t aNumChars = 0;
    intptr_t aNumStrings = 0;
    intptr_t aNumArrays = 0;
    intptr_t aNumObjects = 0;
    intptr_t aNumObjectProps = 0;
    DocTy aResult{theDocInfo};
#define PARSE_STRING(theJson, theNumRead)                                      \
  do {                                                                         \
    std::string_view aStr = p::readString(theJson);                            \
    theNumRead = aStr.size();                                                  \
    aStr = p::stripQuotes(aStr);                                               \
    size_t aNumBytesInStr = 0;                                                 \
    aResult.itsStrings[aNumStrings].itsPosition = aNumChars;                   \
    while (aStr.size()) {                                                      \
      const auto [aChar, aCharWidth] = SourceEncodingTy::decodeFirst(aStr);    \
      if (aCharWidth <= 0)                                                     \
        return makeError<DocTy>("Failed to decode character");                 \
      const auto [aBytes, aBytesUsed] = DestEncodingTy::encode(aChar);         \
      if (aBytesUsed <= 0)                                                     \
        return makeError<DocTy>("Failed to encode character");                 \
      aStr.remove_prefix(aBytesUsed);                                          \
      for (size_t i = 0; i < aBytesUsed; ++i)                                  \
        aResult.itsChars[aNumChars++] = aBytes[i];                             \
      aNumBytesInStr += aBytesUsed;                                            \
    }                                                                          \
    aResult.itsStrings[aNumStrings].itsSize = aNumBytesInStr;                  \
    ++aNumStrings;                                                             \
  } while (false)
    // Invariant: if aCurrentElm is null, aEntity has been set its ElementInfo
    // index before as its payload
    for (Entity &aEntity : aResult.itsEntities) {
      if (!aCurrentElm)
        aCurrentElm = &(*aElementInfos)[aEntity.itsPayload];
      std::string_view aSubJson =
          theJsonString.substr(aCurrentElm->itsLocation);
      // if the parent is an object, consume the key first
      if (aCurrentElm->itsParentId >= 0 &&
          (*aElementInfos)[aCurrentElm->itsParentId].itsType == Type::OBJECT) {
        aResult.itsObjectProps[aNumObjectProps].itsKeyPos = aNumStrings;
        ++aNumObjectProps;
        size_t aLenRead = 0;
        PARSE_STRING(aSubJson, aLenRead);
        aSubJson.remove_prefix(aLenRead);
        aSubJson = p::removeLeadingWhitespace(aSubJson);
        const auto [aChar, aCharWidth] =
            SourceEncodingTy::decodeFirst(aSubJson);
        if (aCharWidth <= 0 || aChar != ':')
          throw std::logic_error("Expected colon");
        aSubJson.remove_prefix(aCharWidth);
        aSubJson = p::removeLeadingWhitespace(aSubJson);
      }

      // now get the value
      switch (aCurrentElm->itsType) {
      case Type::NUL:
        aEntity = {Entity::NUL, 0};
        break;
      case Type::BOOL:
        aEntity = {Entity::BOOL, p::parseBool(aSubJson).first};
        break;
      case Type::NUMBER:
        aEntity = {Entity::NUMBER, aNumNumbers};
        aResult.itsNumbers[aNumNumbers] = p::parseNumber(aSubJson).first;
        ++aNumNumbers;
        break;
      case Type::STRING: {
        aEntity = {Entity::STRING, aNumStrings};
        aResult.itsStrings[aNumStrings].itsPosition = aNumChars;
        PARSE_STRING(aSubJson, std::ignore);
        break;
      }
      case Type::ARRAY:
      case Type::OBJECT: {
        if (aCurrentElm->itsType == Type::OBJECT) {
          aEntity = {Entity::OBJECT, aNumObjects};
          auto &aObj = aResult.itsObjects[aNumObjects];
          aObj.itsKeysPos = aNumPropsAlloced;
          aObj.itsValuesPos = aNumEntitiesAlloced;
          aObj.itsNumProperties = aCurrentElm->itsNumChildren;
          aNumPropsAlloced += aCurrentElm->itsNumChildren;
          ++aNumObjects;
        } else {
          aEntity = {Entity::ARRAY, aNumArrays};
          auto &aArr = aResult.itsArrays[aNumArrays];
          aArr.itsPosition = aNumEntitiesAlloced;
          aArr.itsNumElements = aCurrentElm->itsNumChildren;
          ++aNumArrays;
        }
        // maintain invariant
        if (aCurrentElm->itsFirstChild >= 0) {
          aResult.itsEntities[aNumEntitiesAlloced].itsPayload =
              aCurrentElm->itsFirstChild;
          aNumEntitiesAlloced += aCurrentElm->itsNumChildren;
        }
        break;
      }
      }
      if (aCurrentElm->itsNextSibling != -1) {
        aCurrentElm = &(*aElementInfos)[aCurrentElm->itsNextSibling];
      } else {
        aCurrentElm = nullptr;
      }
    }
#undef PARSE_STRING
    return aResult;
  }

private:
  template <typename DocTy>
  using ElementInfos = typename DocTy::Storage::Buffer<ElementInfo, DocTy::Storage::MAX_ENTITIES()>;

  static constexpr std::string_view
  consumeObjectKey(const std::string_view theString) {
    std::string_view aRemaining = theString;
    aRemaining.remove_prefix(p::readString(aRemaining).size());
    aRemaining = p::removeLeadingWhitespace(aRemaining);
    const auto [aChar, aCharWidth] = SourceEncodingTy::decodeFirst(aRemaining);
    if (aCharWidth <= 0 || aChar != ':')
      throw std::logic_error("Expected colon");
    aRemaining.remove_prefix(aCharWidth);
    aRemaining = p::removeLeadingWhitespace(aRemaining);
    return aRemaining;
  }

  template <typename DocTy>
  static constexpr auto
  computeElementInfos(const std::string_view theJsonString,
                      const DocumentInfo &theDocInfo)
      -> std::optional<ElementInfos<DocTy>> {
    // state variables
    ElementInfos<DocTy> aEntities{
        DocTy::Storage::template createBuffer<ElementInfo,
                                              DocTy::Storage::MAX_ENTITIES()>(
            static_cast<size_t>(theDocInfo.itsNumArrayEntries +
                                theDocInfo.itsNumObjectProperties + 1))};
    ParentId aCurrentParent = -1;
    Type aCurrentParentType = Type::NUL;
    bool aIsFirstChild = true;
    std::string_view aRemaining = theJsonString;

    for (size_t aElmIdx = 0; aElmIdx < aEntities.size(); ++aElmIdx) {
      aRemaining = p::removeLeadingWhitespace(aRemaining);
      // leave surrounding element(s)
      for (;;) {
        const auto [aChar, aCharWidth] =
            SourceEncodingTy::decodeFirst(aRemaining);
        if (aCharWidth <= 0)
          return makeElmInfoError<DocTy>(
              "Unexpected EOF or failed to decode character");
        if ((aCurrentParentType == Type::OBJECT && aChar == '}') ||
            (aCurrentParentType == Type::ARRAY && aChar == ']')) {
          // move one level up again
          aRemaining.remove_prefix(aCharWidth);
          aRemaining = p::removeLeadingWhitespace(aRemaining);
          const auto &aOldParentInfo = aEntities[aCurrentParent];
          aCurrentParent = aOldParentInfo.itsParentId;
          aIsFirstChild = false;
          if (aCurrentParent >= 0) {
            const auto &aNewParentInfo = aEntities[aCurrentParent];
            if (aNewParentInfo.itsType == Type::OBJECT ||
                aNewParentInfo.itsType == Type::ARRAY)
              aCurrentParentType = aNewParentInfo.itsType;
            else
              throw std::logic_error{
                  "Parent element is neither array nor object"};
          } else {
            aCurrentParentType = Type::NUL;
            break;
          }
          continue;
        }
        break;
      }
      // sanity check
      if (!aIsFirstChild && aCurrentParentType == Type::NUL) {
        throw std::logic_error{
            "Second element in root scope. You may be using the wrong DocType"};
      }
      // consume comma
      if (!aIsFirstChild) {
        const auto [aChar, aCharWidth] =
            SourceEncodingTy::decodeFirst(aRemaining);
        if (aChar != ',')
          return makeElmInfoError<DocTy>("Expected comma");
        aRemaining.remove_prefix(aCharWidth);
        aRemaining = p::removeLeadingWhitespace(aRemaining);
      }

      // location is registered before the key is consumed
      const Location aLocation = theJsonString.size() - aRemaining.size();

      // consume key
      if (aCurrentParentType == Type::OBJECT) {
        aRemaining = consumeObjectKey(aRemaining);
      }
      // detect type
      auto aTypeOpt = p::detectElementType(aRemaining);
      if (!aTypeOpt) {
        return makeElmInfoError<DocTy>("Failed to detect element type");
      }

      // set element properties
      aEntities[aElmIdx].itsId = aElmIdx;
      aEntities[aElmIdx].itsType = *aTypeOpt;
      aEntities[aElmIdx].itsParentId = aCurrentParent;
      aEntities[aElmIdx].itsLocation = aLocation;
      if (aCurrentParent >= 0) {
        auto &aParent = aEntities[aCurrentParent];
        ++aParent.itsNumChildren;
        if (aIsFirstChild) {
          aParent.itsFirstChild = aElmIdx;
        }
        const auto aPrevSibIdx = aParent.itsLastChild;
        aParent.itsLastChild = aElmIdx;
        if (aPrevSibIdx != -1) {
          aEntities[aPrevSibIdx].itsNextSibling = aElmIdx;
        }
      }
      // setup for reading next element
      aIsFirstChild = false;
      switch (*aTypeOpt) {
      case Type::NUL:
        aRemaining.remove_prefix(p::readNull(aRemaining).size());
        break;
      case Type::BOOL:
        aRemaining.remove_prefix(p::parseBool(aRemaining).second);
        break;
      case Type::NUMBER:
        aRemaining.remove_prefix(p::readNumber(aRemaining).size());
        break;
      case Type::STRING:
        aRemaining.remove_prefix(p::readString(aRemaining).size());
        break;
      case Type::ARRAY:
      case Type::OBJECT: {
        if (*aTypeOpt == Type::ARRAY) {
          aRemaining.remove_prefix(SourceEncodingTy::encode('[').second);
          aCurrentParentType = Type::ARRAY;
        } else {
          aRemaining.remove_prefix(SourceEncodingTy::encode('{').second);
          aCurrentParentType = Type::OBJECT;
        }
        aCurrentParent = aElmIdx;
        aIsFirstChild = true;
        break;
      }
      }
    }
    return aEntities;
  }

  template <typename DocTy>
  static constexpr auto makeElmInfoError(const char *const theMsg)
      -> std::optional<ElementInfos<DocTy>> {
    throw std::invalid_argument(theMsg);
    return std::nullopt;
  }
  template <typename DocTy>
  static constexpr std::optional<DocTy> makeError(const char *const theMsg) {
    throw std::invalid_argument(theMsg);
    return std::nullopt;
  }
}; // namespace cjson
} // namespace cjson
#endif // CONSTEXPR_JSON_DOCUMENT_BUILDER2_H
