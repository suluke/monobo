#ifndef CONSTEXPR_JSON_DOCUMENT_PARSER1_H
#define CONSTEXPR_JSON_DOCUMENT_PARSER1_H

#include "constexpr_json/document.h"
#include "constexpr_json/document_info.h"
#include "constexpr_json/impl/parsing_utils.h"

#include <cassert>

namespace cjson {
template <typename SourceEncodingTy, typename DestEncodingTy,
          typename ErrorHandlingTy>
struct DocumentParser1 {
  template <typename DocTy>
  constexpr static DocTy createNullDocument(const DocumentInfo &theDocInfo) {
    DocTy aResult{theDocInfo};
    aResult.itsEntities[0] = Entity{Entity::NUL, 0};
    return aResult;
  }
  template <typename DocTy>
  constexpr static DocTy createBoolDocument(bool theValue,
                                            const DocumentInfo &theDocInfo) {
    DocTy aResult{theDocInfo};
    aResult.itsEntities[0] = Entity{Entity::BOOL, theValue};
    return aResult;
  }
  template <typename DocTy>
  constexpr static DocTy createNumberDocument(double theValue,
                                              const DocumentInfo &theDocInfo) {
    DocTy aResult{theDocInfo};
    aResult.itsEntities[0] = Entity{Entity::NUMBER, 0};
    aResult.itsNumbers[0] = theValue;
    return aResult;
  }
  constexpr static bool
  isNonAggregateDocumentType(const DocumentInfo &theDocInfo) {
    return theDocInfo.itsNumNumbers == 0 && theDocInfo.itsNumStrings == 0 &&
           theDocInfo.itsNumArrays == 0 && theDocInfo.itsNumArrayEntries == 0 &&
           theDocInfo.itsNumObjects == 0 &&
           theDocInfo.itsNumObjectProperties == 0;
  }
  constexpr static bool isNumberDocumentType(const DocumentInfo &theDocInfo) {
    return theDocInfo.itsNumNumbers == 1 && theDocInfo.itsNumStrings == 0 &&
           theDocInfo.itsNumArrays == 0 && theDocInfo.itsNumArrayEntries == 0 &&
           theDocInfo.itsNumObjects == 0 &&
           theDocInfo.itsNumObjectProperties == 0;
  }
  constexpr static bool isStringDocumentType(const DocumentInfo &theDocInfo) {
    return theDocInfo.itsNumNumbers == 0 && theDocInfo.itsNumStrings == 1 &&
           theDocInfo.itsNumArrays == 0 && theDocInfo.itsNumArrayEntries == 0 &&
           theDocInfo.itsNumObjects == 0 &&
           theDocInfo.itsNumObjectProperties == 0;
  }

  template <typename DocTy>
  using ResultTy = typename ErrorHandlingTy::template ErrorOr<DocTy>;

  template <typename DocTy>
  constexpr static ResultTy<DocTy>
  parseDocument(const std::string_view theJsonString,
                const DocumentInfo &theDocInfo) {
    const ResultTy<DocTy> aErrorResult =
        ErrorHandlingTy::template makeError<DocTy>(ErrorCode::UNKNOWN, -1);
    using p = parsing<SourceEncodingTy>;
    using Type = typename p::Type;
    const std::string_view aWS = p::readWhitespace(theJsonString);
    std::string_view aRemaining = theJsonString.substr(aWS.size());
    std::optional<Type> aType = p::detectElementType(aRemaining);
    if (!aType)
      return aErrorResult;
    if (isNonAggregateDocumentType(theDocInfo)) {
      switch (*aType) {
      case Type::NUL: {
        const std::string_view aNull = p::readNull(aRemaining);
        if (aNull.size() == 0)
          return aErrorResult;
        aRemaining.remove_prefix(aNull.size());
        return createNullDocument<DocTy>(theDocInfo);
      }
      case Type::BOOL: {
        const auto [aBoolVal, aBoolLen] = p::parseBool(aRemaining);
        if (aBoolLen <= 0)
          return aErrorResult;
        aRemaining.remove_prefix(aBoolLen);
        return createBoolDocument<DocTy>(aBoolVal, theDocInfo);
      }
      default:
        return aErrorResult;
      }
    } else if (isNumberDocumentType(theDocInfo)) {
      if (*aType == Type::NUMBER) {
        const auto [aNumVal, aNumLen] = p::parseNumber(aRemaining);
        if (aNumLen <= 0)
          return aErrorResult;
        aRemaining.remove_prefix(aNumLen);
        return createNumberDocument<DocTy>(aNumVal, theDocInfo);
      }
      return aErrorResult;
    } else if (isStringDocumentType(theDocInfo)) {
      if (*aType == Type::STRING) {
        const std::string_view aStrQuot = p::readString(aRemaining);
        if (aStrQuot.empty())
          return aErrorResult;
        aRemaining.remove_prefix(aStrQuot.size());
        std::string_view aStr = p::stripQuotes(aStrQuot);
        DocTy aStrDoc{theDocInfo};
        for (size_t i = 0; i < static_cast<size_t>(theDocInfo.itsNumChars);) {
          const auto [aChar, aCharWidth] = p::parseFirstStringChar(aStr);
          if (aCharWidth <= 0)
            return aErrorResult;
          const auto [aBytes, aBytesNum] = DestEncodingTy::encode(aChar);
          if (aBytesNum <= 0)
            return aErrorResult;
          aStr.remove_prefix(aCharWidth);
          for (size_t j = 0;
               j < aBytesNum && i < static_cast<size_t>(theDocInfo.itsNumChars);
               ++i, ++j)
            aStrDoc.itsChars[i] = aBytes[j];
        }
        aStrDoc.itsStrings[0] =
            String{0, static_cast<size_t>(theDocInfo.itsNumChars)};
        aStrDoc.itsEntities[0] = Entity{Entity::STRING, 0};
        return aStrDoc;
      }
      return aErrorResult;
    } else {
      // Idea: Detect the types of the direct child elements of the
      // root-array/object and store them in the result DocTy. However, only
      // store their position inside the JSON string as the payload. Then
      // re-iterate the entities array from left to right doing the same until
      // the whole document is built up.
      DocTy aResult{theDocInfo};
      aResult.itsEntities[0].itsPayload = 0;
      switch (*aType) {
      case Type::ARRAY:
        aResult.itsEntities[0].itsKind = Entity::ARRAY;
        break;
      case Type::OBJECT:
        aResult.itsEntities[0].itsKind = Entity::OBJECT;
        break;
      default:
        return aErrorResult;
      }
      intptr_t aNumEntities = 1;
      intptr_t aNumNumbers = 0;
      intptr_t aNumChars = 0;
      intptr_t aNumStrings = 0;
      intptr_t aNumArrays = 0;
      intptr_t aNumObjects = 0;
      intptr_t aNumObjectProps = 0;
      const auto getEntityKindFromParsingType =
          [](Type theType) -> Entity::KIND {
        switch (theType) {
        case Type::NUL:
          return Entity::NUL;
        case Type::BOOL:
          return Entity::BOOL;
        case Type::NUMBER:
          return Entity::NUMBER;
        case Type::STRING:
          return Entity::STRING;
        case Type::ARRAY:
          return Entity::ARRAY;
        case Type::OBJECT:
          return Entity::OBJECT;
        }
        return Entity::NUL;
      };
      for (Entity &aEntity : aResult.itsEntities) {
        const std::string_view aEntityStr =
            aRemaining.substr(aEntity.itsPayload);
        switch (aEntity.itsKind) {
        case Entity::NUL: {
          std::string_view aNull = p::readNull(aEntityStr);
          if (aNull.size() == 0)
            return aErrorResult;
          aEntity.itsPayload = 0;
          break;
        }
        case Entity::BOOL: {
          const auto [aBool, aBoolWidth] = p::parseBool(aEntityStr);
          if (aBoolWidth <= 0)
            return aErrorResult;
          aEntity.itsPayload = aBool;
          break;
        }
        case Entity::NUMBER: {
          const auto [aNumber, aNumberWidth] = p::parseNumber(aEntityStr);
          if (aNumberWidth <= 0)
            return aErrorResult;
          aResult.itsNumbers[aNumNumbers] = aNumber;
          aEntity.itsPayload = aNumNumbers;
          ++aNumNumbers;
          break;
        }
        case Entity::STRING: {
          std::string_view aStr = p::stripQuotes(p::readString(aEntityStr));
          aEntity.itsPayload = aNumStrings;
          size_t aNumBytesInStr = 0;
          aResult.itsStrings[aNumStrings].itsPosition = aNumChars;
          while (!aStr.empty()) {
            const auto [aChar, aCharWidth] =
                SourceEncodingTy::decodeFirst(aStr);
            if (aCharWidth <= 0)
              return aErrorResult;
            const auto [aBytes, aBytesUsed] = DestEncodingTy::encode(aChar);
            if (aBytesUsed <= 0)
              return aErrorResult;
            aStr.remove_prefix(aBytesUsed);
            for (size_t i = 0; i < aBytesUsed; ++i)
              aResult.itsChars[aNumChars++] = aBytes[i];
            aNumBytesInStr += aBytesUsed;
          }
          aResult.itsStrings[aNumStrings].itsSize = aNumBytesInStr;
          ++aNumStrings;
          break;
        }
        case Entity::ARRAY:
          [[fallthrough]];
        case Entity::OBJECT: {
          std::string_view aAggregateJson = aEntityStr;
          const auto aDecoded = SourceEncodingTy::decodeFirst(aAggregateJson);
          using CharT = typename SourceEncodingTy::CodePointTy;
          const bool isObject = aEntity.itsKind == Entity::OBJECT;
          const CharT aStopChar = isObject ? '}' : ']';
          assert(aDecoded.first == (isObject ? '{' : '[') &&
                 "Expected aggregate position to start on '['/'{'");
          aAggregateJson.remove_prefix(aDecoded.second);
          bool aNeedsElement = false;
          intptr_t aNumChildren = 0;
          if (isObject) {
            aEntity.itsPayload = aNumObjects++;
            aResult.itsObjects[aEntity.itsPayload].itsKeysPos = aNumObjectProps;
            aResult.itsObjects[aEntity.itsPayload].itsValuesPos = aNumEntities;
          } else {
            aEntity.itsPayload = aNumArrays++;
            aResult.itsArrays[aEntity.itsPayload].itsPosition = aNumEntities;
          }
          for (;;) {
            {
              aAggregateJson.remove_prefix(
                  p::readWhitespace(aAggregateJson).size());
              const auto aDecoded =
                  SourceEncodingTy::decodeFirst(aAggregateJson);
              assert(aDecoded.second > 0 &&
                     "Failed to decode char where one was expected");
              if (!aNeedsElement && aDecoded.first == aStopChar)
                break;
            }
            if (isObject) {
              std::string_view aKeyStr = p::readString(aAggregateJson);
              if (aKeyStr.size() <= 0)
                return aErrorResult;
              aAggregateJson.remove_prefix(aKeyStr.size());
              aAggregateJson.remove_prefix(
                  p::readWhitespace(aAggregateJson).size());
              const auto [aColon, aColonWidth] =
                  SourceEncodingTy::decodeFirst(aAggregateJson);
              if (aColonWidth <= 0)
                return aErrorResult;
              if (aColon != ':')
                return aErrorResult;
              aAggregateJson.remove_prefix(aColonWidth);
              aAggregateJson.remove_prefix(
                  p::readWhitespace(aAggregateJson).size());
              // Save key into strings
              // TODO this is mostly duplicated code
              aKeyStr = p::stripQuotes(aKeyStr);
              aResult.itsObjectProps[aNumObjectProps].itsKeyPos = aNumStrings;
              size_t aNumBytesInStr = 0;
              aResult.itsStrings[aNumStrings].itsPosition = aNumChars;
              while (!aKeyStr.empty()) {
                const auto [aChar, aCharWidth] =
                    SourceEncodingTy::decodeFirst(aKeyStr);
                if (aCharWidth <= 0)
                  return aErrorResult;
                const auto [aBytes, aBytesUsed] = DestEncodingTy::encode(aChar);
                if (aBytesUsed <= 0)
                  return aErrorResult;
                aKeyStr.remove_prefix(aBytesUsed);
                for (size_t i = 0; i < aBytesUsed; ++i)
                  aResult.itsChars[aNumChars++] = aBytes[i];
                aNumBytesInStr += aBytesUsed;
              }
              aResult.itsStrings[aNumStrings].itsSize = aNumBytesInStr;
              ++aNumStrings;
              ++aNumObjectProps;
            }
            std::optional<std::pair<Type, std::string_view>> aElmMaybe =
                p::readElement(aAggregateJson);
            if (!aElmMaybe)
              return aErrorResult;
            // It seems in gcc7 optional::operator-> is not constexpr?
            std::string_view aElmStr = (*aElmMaybe).second;
            aAggregateJson.remove_prefix(aElmStr.size());
            aResult.itsEntities[aNumEntities] = {
                getEntityKindFromParsingType((*aElmMaybe).first),
                static_cast<intptr_t>(aRemaining.size() -
                                      aAggregateJson.size() - aElmStr.size())};
            ++aNumChildren;
            ++aNumEntities;
            aAggregateJson.remove_prefix(
                p::readWhitespace(aAggregateJson).size());
            {
              const auto [aChar, aCharWidth] =
                  SourceEncodingTy::decodeFirst(aAggregateJson);
              assert(aCharWidth > 0 &&
                     "Failed to decode char where one was expected");
              if (aChar == ',') {
                aAggregateJson.remove_prefix(aCharWidth);
                aNeedsElement = true;
              } else {
                aNeedsElement = false;
              }
            }
          }
          if (isObject) {
            aResult.itsObjects[aEntity.itsPayload].itsNumProperties =
                aNumChildren;
          } else {
            aResult.itsArrays[aEntity.itsPayload].itsNumElements = aNumChildren;
          }
          break;
        }
        default:
          return aErrorResult;
        }
      }
      return aResult;
    }
    return aErrorResult;
  }
};
} // namespace cjson
#endif // CONSTEXPR_JSON_DOCUMENT_PARSER1_H
