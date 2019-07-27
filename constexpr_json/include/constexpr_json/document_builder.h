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
      this->itsNumArrayEntries += theOther.itsNumArrayEntries;
      this->itsNumArrays += theOther.itsNumArrays;
      this->itsNumBools += theOther.itsNumBools;
      this->itsNumChars += theOther.itsNumChars;
      this->itsNumNumbers += theOther.itsNumNumbers;
      this->itsNumNulls += theOther.itsNumNulls;
      this->itsNumObjectProperties += theOther.itsNumObjectProperties;
      this->itsNumObjects += theOther.itsNumObjects;
      this->itsNumStrings += theOther.itsNumStrings;
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
        itsType = Entity::BOOLEAN;
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
        itsDocInfo.itsNumChars += theKey.size();
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
    aResult.itsRoot = Entity{Entity::NUL, 0};
    return aResult;
  }
  constexpr static NonAggregateDocument createBoolDocument(bool theValue) {
    NonAggregateDocument aResult;
    aResult.itsRoot = Entity{Entity::BOOLEAN, theValue};
    return aResult;
  }
  constexpr static NumberDocument createNumberDocument(double theValue) {
    NumberDocument aResult;
    aResult.itsRoot = Entity{Entity::NUMBER, 0};
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
        aStrDoc.itsRoot = Entity{Entity::STRING, 0};
        return std::make_pair(aStrDoc,
                              theJsonString.size() - aRemaining.size());
      }
      return aErrorResult;
    } else {
      switch (*aType) {
      case Type::ARRAY: {
        const auto [aBracket, aBracketWidth] = SourceEncodingTy::decodeFirst(aRemaining);
        if (aBracket != '[')
          return aErrorResult;
        aRemaining.remove_prefix(aBracketWidth);
        // TODO
      }
      case Type::OBJECT:
      default:
        return aErrorResult;
      }
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
