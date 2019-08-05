#ifndef CONSTEXPR_JSON_DOCUMENT_BUILDER_H
#define CONSTEXPR_JSON_DOCUMENT_BUILDER_H

#include "document.h"
#include "utils/parsing.h"
#include "utils/unicode.h"
#include <memory>
#include <optional>

namespace cjson {
template <typename SourceEncodingTy, typename DestEncodingTy>
struct DocumentBuilder {
  struct DocumentInfo {
    ssize_t itsNumNulls = 0;
    ssize_t itsNumBools = 0;
    ssize_t itsNumNumbers = 0;
    ssize_t itsNumChars = 0;
    ssize_t itsNumStrings = 0;
    ssize_t itsNumArrays = 0;
    ssize_t itsNumArrayEntries = 0;
    ssize_t itsNumObjects = 0;
    ssize_t itsNumObjectProperties = 0;

    constexpr static DocumentInfo error() {
      DocumentInfo aDI{-1, -1, -1, -1, -1, -1, -1, -1, -1};
      return aDI;
    }

    constexpr operator bool() const {
      return itsNumNulls >= 0 && itsNumBools >= 0 && itsNumNumbers >= 0 &&
             itsNumChars >= 0 && itsNumStrings >= 0 && itsNumArrays >= 0 &&
             itsNumArrayEntries >= 0 && itsNumObjects >= 0 &&
             itsNumObjectProperties >= 0;
    }
    constexpr DocumentInfo operator+(const DocumentInfo &theOther) {
      DocumentInfo aDI = *this;
      aDI += theOther;
      return aDI;
    }
    constexpr DocumentInfo &operator+=(const DocumentInfo &theOther) {
      itsNumArrayEntries += theOther.itsNumArrayEntries;
      itsNumArrays += theOther.itsNumArrays;
      itsNumBools += theOther.itsNumBools;
      itsNumChars += theOther.itsNumChars;
      itsNumNumbers += theOther.itsNumNumbers;
      itsNumNulls += theOther.itsNumNulls;
      itsNumObjectProperties += theOther.itsNumObjectProperties;
      itsNumObjects += theOther.itsNumObjects;
      itsNumStrings += theOther.itsNumStrings;
      return *this;
    }
    constexpr bool operator==(const DocumentInfo &theOther) const {
      return itsNumArrayEntries == theOther.itsNumArrayEntries &&
             itsNumArrays == theOther.itsNumArrays &&
             itsNumBools == theOther.itsNumBools &&
             itsNumChars == theOther.itsNumChars &&
             itsNumNumbers == theOther.itsNumNumbers &&
             itsNumNulls == theOther.itsNumNulls &&
             itsNumObjectProperties == theOther.itsNumObjectProperties &&
             itsNumObjects == theOther.itsNumObjects &&
             itsNumStrings == theOther.itsNumStrings;
    }
  };

  constexpr static DocumentInfo
  computeDocInfo(const std::string_view theJsonString) {
    constexpr const DocumentInfo aErrorResult = DocumentInfo::error();
    struct InfoElement {
      Entity::KIND itsType = Entity::NUL;
      DocumentInfo itsDocInfo = {};
      constexpr void setBool(bool theBool) {
        itsType = Entity::BOOL;
        ++itsDocInfo.itsNumBools;
      }
      constexpr void setNumber(double theNum) {
        itsType = Entity::NUMBER;
        ++itsDocInfo.itsNumNumbers;
      }
      constexpr void setString(std::string_view theStr) {
        itsType = Entity::STRING;
        ++itsDocInfo.itsNumStrings;
        // Count the required number of bytes in the output encoding
        size_t aNumChars = 0;
        while (!theStr.empty()) {
          const auto [aChar, aCharWidth] =
              SourceEncodingTy::decodeFirst(theStr);
          if (aChar == '\\') {
            const auto [aCodepoint, aWidth] =
                parsing<SourceEncodingTy>::parseEscape(theStr);
            theStr.remove_prefix(aWidth);
            aNumChars += DestEncodingTy::encode(aCodepoint).second;
          } else {
            theStr.remove_prefix(aCharWidth);
            aNumChars += DestEncodingTy::encode(aChar).second;
          }
        }
        itsDocInfo.itsNumChars += aNumChars;
      }
      constexpr void setNull() {
        itsType = Entity::NUL;
        ++itsDocInfo.itsNumNulls;
      }
      constexpr void setArray() {
        itsType = Entity::ARRAY;
        ++itsDocInfo.itsNumArrays;
      }
      constexpr void addArrayEntry(const InfoElement &theEntry) {
        ++itsDocInfo.itsNumArrayEntries;
        itsDocInfo += theEntry.itsDocInfo;
      }
      constexpr void setObject() {
        itsType = Entity::OBJECT;
        ++itsDocInfo.itsNumObjects;
      }
      constexpr void addObjectProperty(const std::string_view theKey,
                                       const InfoElement &theValue) {
        ++itsDocInfo.itsNumObjectProperties;
        // Count the required number of bytes in the output encoding
        size_t aNumChars = 0;
        std::string_view aRemaining = theKey;
        while (!aRemaining.empty()) {
          const auto [aChar, aCharWidth] =
              SourceEncodingTy::decodeFirst(aRemaining);
          if (aChar == '\\') {
            const auto [aCodepoint, aWidth] =
                parsing<SourceEncodingTy>::parseEscape(aRemaining);
            aRemaining.remove_prefix(aWidth);
            aNumChars += DestEncodingTy::encode(aCodepoint).second;
          } else {
            aRemaining.remove_prefix(aCharWidth);
            aNumChars += DestEncodingTy::encode(aChar).second;
          }
        }
        itsDocInfo.itsNumChars += aNumChars;
        ++itsDocInfo.itsNumStrings;
        itsDocInfo += theValue.itsDocInfo;
      }
      constexpr static InfoElement null() { return InfoElement{}; }
    };
    const auto [aParsed, aParsedLen] =
        parsing<SourceEncodingTy>::template parseElement<InfoElement>(
            theJsonString);
    if (aParsedLen <= 0)
      return aErrorResult;
    return aParsed.itsDocInfo;
  }

  using NonAggregateDocument = Document<0, 0, 0, 0, 0, 0, 0>;
  using NumberDocument = Document<1, 0, 0, 0, 0, 0, 0>;
  template <size_t NumChars>
  using StringDocument = Document<0, NumChars, 1, 0, 0, 0, 0>;

  constexpr static NonAggregateDocument createNullDocument() {
    NonAggregateDocument aResult;
    aResult.itsEntities[0] = Entity{Entity::NUL, 0};
    return aResult;
  }
  constexpr static NonAggregateDocument createBoolDocument(bool theValue) {
    NonAggregateDocument aResult;
    aResult.itsEntities[0] = Entity{Entity::BOOL, theValue};
    return aResult;
  }
  constexpr static NumberDocument createNumberDocument(double theValue) {
    NumberDocument aResult;
    aResult.itsEntities[0] = Entity{Entity::NUMBER, 0};
    aResult.itsNumbers[0] = theValue;
    return aResult;
  }
  template <typename DocTy> constexpr static bool isStringDocumentType() {
    return DocTy::itsNumNumbers == 0 && DocTy::itsNumStrings == 1 &&
           DocTy::itsNumArrays == 0 && DocTy::itsNumArrayEntries == 0 &&
           DocTy::itsNumObjects == 0 && DocTy::itsNumObjectProperties == 0;
  }

  template <typename DocTy>
  constexpr static std::pair<std::optional<DocTy>, ssize_t>
  parseDocument(const std::string_view theJsonString) {
    constexpr const auto aErrorResult =
        std::make_pair<std::optional<DocTy>, ssize_t>(std::nullopt, -1);
    using p = parsing<SourceEncodingTy>;
    using Type = typename p::Type;
    const std::string_view aWS = p::readWhitespace(theJsonString);
    std::string_view aRemaining = theJsonString.substr(aWS.size());
    std::optional<Type> aType = p::detectElementType(aRemaining);
    if (!aType)
      return aErrorResult;
    if constexpr (std::is_same_v<DocTy, NonAggregateDocument>) {
      switch (*aType) {
      case Type::NUL: {
        const ssize_t aNullLen = p::parseNull(aRemaining);
        if (aNullLen <= 0)
          return aErrorResult;
        aRemaining.remove_prefix(aNullLen);
        return std::make_pair(createNullDocument(),
                              theJsonString.size() - aRemaining.size());
      }
      case Type::BOOL: {
        const auto [aBoolVal, aBoolLen] = p::parseBool(aRemaining);
        if (aBoolLen <= 0)
          return aErrorResult;
        aRemaining.remove_prefix(aBoolLen);
        return std::make_pair(createBoolDocument(aBoolVal),
                              theJsonString.size() - aRemaining.size());
      }
      default:
        return aErrorResult;
      }
    } else if constexpr (std::is_same_v<DocTy, NumberDocument>) {
      if (*aType == Type::NUMBER) {
        const auto [aNumVal, aNumLen] = p::parseNumber(aRemaining);
        if (aNumLen <= 0)
          return aErrorResult;
        aRemaining.remove_prefix(aNumLen);
        return std::make_pair(createNumberDocument(aNumVal),
                              theJsonString.size() - aRemaining.size());
      }
      return aErrorResult;
    } else if constexpr (isStringDocumentType<DocTy>()) {
      if (*aType == Type::STRING) {
        const std::string_view aStrQuot = p::readString(aRemaining);
        if (aStrQuot.empty())
          return aErrorResult;
        aRemaining.remove_prefix(aStrQuot.size());
        std::string_view aStr = p::stripQuotes(aStrQuot);
        DocTy aStrDoc;
        for (size_t i = 0; i < DocTy::itsNumChars;) {
          const auto [aChar, aCharWidth] = p::parseFirstStringChar(aStr);
          if (aCharWidth <= 0)
            return aErrorResult;
          const auto [aBytes, aBytesNum] = DestEncodingTy::encode(aChar);
          if (aBytesNum <= 0)
            return aErrorResult;
          aStr.remove_prefix(aCharWidth);
          for (int j = 0; j < aBytesNum && i < DocTy::itsNumChars; ++i, ++j)
            aStrDoc.itsChars[i] = aBytes[j];
        }
        aStrDoc.itsStrings[0] = String{0, DocTy::itsNumChars};
        aStrDoc.itsEntities[0] = Entity{Entity::STRING, 0};
        return std::make_pair(aStrDoc,
                              theJsonString.size() - aRemaining.size());
      }
      return aErrorResult;
    } else {
      // Idea: Detect the types of the direct child elements of the
      // root-array/object and store them in the result DocTy. However, only
      // store their position inside the JSON string as the payload. Then
      // re-iterate the entities array from left to right doing the same until
      // the whole document is built up.
      DocTy aResult;
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
      ssize_t aNumEntities = 1;
      ssize_t aNumNumbers = 0;
      ssize_t aNumChars = 0;
      ssize_t aNumStrings = 0;
      ssize_t aNumArrays = 0;
      ssize_t aNumObjects = 0;
      ssize_t aNumObjectProps = 0;
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
          ssize_t aNullLen = p::parseNull(aEntityStr);
          if (aNullLen <= 0)
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
            for (ssize_t i = 0; i < aBytesUsed; ++i)
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
          const auto [aBracket, aBracketWidth] =
              SourceEncodingTy::decodeFirst(aAggregateJson);
          using CharT = typename SourceEncodingTy::CodePointTy;
          const bool isObject = aEntity.itsKind == Entity::OBJECT;
          const CharT aStartChar = isObject ? '{' : '[';
          const CharT aStopChar = isObject ? '}' : ']';
          if (aBracket != aStartChar)
            throw std::logic_error{
                "Expected aggregate position to start on '['/'{'"};
          aAggregateJson.remove_prefix(aBracketWidth);
          bool aNeedsElement = false;
          ssize_t aNumChildren = 0;
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
              const auto [aChar, aCharWidth] =
                  SourceEncodingTy::decodeFirst(aAggregateJson);
              if (aCharWidth <= 0)
                throw std::logic_error{
                    "Failed to decode char where one was expected"};
              if (!aNeedsElement && aChar == aStopChar)
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
                for (ssize_t i = 0; i < aBytesUsed; ++i)
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
            std::string_view aElmStr = aElmMaybe->second;
            aAggregateJson.remove_prefix(aElmStr.size());
            aResult.itsEntities[aNumEntities] = {
                getEntityKindFromParsingType(aElmMaybe->first),
                static_cast<intptr_t>(aRemaining.size() -
                                      aAggregateJson.size() - aElmStr.size())};
            ++aNumChildren;
            ++aNumEntities;
            aAggregateJson.remove_prefix(
                p::readWhitespace(aAggregateJson).size());
            {
              const auto [aChar, aCharWidth] =
                  SourceEncodingTy::decodeFirst(aAggregateJson);
              if (aCharWidth <= 0)
                throw std::logic_error{
                    "Failed to decode char where one was expected"};
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
      return std::make_pair(aResult, theJsonString.size());
    }
    return aErrorResult;
  }

  static std::unique_ptr<DocumentBase>
  buildDocument(const std::string_view theJsonString) {
    constexpr DocumentInfo aDocInfo = computeDocInfo(theJsonString);
    if (!aDocInfo)
      return nullptr;
    using DocTy = Document<aDocInfo.itsNumNumbers, aDocInfo.itsNumChars,
                           aDocInfo.itsNumStrings, aDocInfo.itsNumArrays,
                           aDocInfo.itsNumArrayEntries, aDocInfo.itsNumObjects,
                           aDocInfo.itsNumObjectProperties>;
    std::optional<DocTy> aParseResult = parseDocument<DocTy>(theJsonString);
    if (!aParseResult) {
      return nullptr;
    }
    std::unique_ptr<DocTy> aDoc = std::make_unique<DocTy>(*aParseResult);
    return aDoc;
  }
};
} // namespace cjson
#endif // CONSTEXPR_JSON_DOCUMENT_BUILDER_H
