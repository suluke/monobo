#ifndef CONSTEXPR_JSON_DOCUMENT_INFO_H
#define CONSTEXPR_JSON_DOCUMENT_INFO_H

#include <stdexcept>
#include "utils/parsing.h"

namespace cjson {
struct DocumentInfo {
  ssize_t itsNumNulls = 0;
  ssize_t itsNumBools = 0;
  ssize_t itsNumNumbers = 0;
  /// The number of chars (bytes) required to store all encoded string data.
  /// NOTE: '\n' e.g. takes 2 chars in the source json string but only 1 in
  /// the encoded string data.
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
  constexpr static DocumentInfo SingleNull() {
    return {1, 0, 0, 0, 0, 0, 0, 0, 0};
  }
  constexpr static DocumentInfo SingleBool() {
    return {0, 1, 0, 0, 0, 0, 0, 0, 0};
  }
  constexpr static DocumentInfo SingleNumber() {
    return {0, 0, 1, 0, 0, 0, 0, 0, 0};
  }
  constexpr static DocumentInfo SingleString(ssize_t theNumChars) {
    return {0, 0, 0, theNumChars, 1, 0, 0, 0, 0};
  }
  constexpr static DocumentInfo EmptyArray() {
    return {0, 0, 0, 0, 0, 1, 0, 0, 0};
  }
  constexpr static DocumentInfo EmptyObject() {
    return {0, 0, 0, 0, 0, 0, 0, 1, 0};
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
  constexpr bool assertSame(const DocumentInfo &theOther) const {
    if (itsNumArrayEntries != theOther.itsNumArrayEntries)
      throw std::invalid_argument(
          "DocumentInfo mismatch at itsNumArrayEntries");
    if (itsNumArrays != theOther.itsNumArrays)
      throw std::invalid_argument("DocumentInfo mismatch at itsNumArrays");
    if (itsNumBools != theOther.itsNumBools)
      throw std::invalid_argument("DocumentInfo mismatch at itsNumBools");
    if (itsNumChars != theOther.itsNumChars)
      throw std::invalid_argument("DocumentInfo mismatch at itsNumChars");
    if (itsNumNumbers != theOther.itsNumNumbers)
      throw std::invalid_argument("DocumentInfo mismatch at itsNumNumbers");
    if (itsNumNulls != theOther.itsNumNulls)
      throw std::invalid_argument("DocumentInfo mismatch at itsNumNulls");
    if (itsNumObjectProperties != theOther.itsNumObjectProperties)
      throw std::invalid_argument(
          "DocumentInfo mismatch at itsNumObjectProperties");
    if (itsNumObjects != theOther.itsNumObjects)
      throw std::invalid_argument("DocumentInfo mismatch at itsNumObjects");
    if (itsNumStrings != theOther.itsNumStrings)
      throw std::invalid_argument("DocumentInfo mismatch at itsNumStrings");
    return true;
  }

  /**
   * @return .first is the DocInfo, .second is the number of read chars (i.e.
   * bytes, not code points)
   */
  template <typename SourceEncodingTy, typename DestEncodingTy>
  constexpr static std::pair<DocumentInfo, ssize_t>
  compute(const std::string_view theJsonString) {
    // setup
    using p = parsing<SourceEncodingTy>;
    using Type = typename p::Type;
    std::string_view aRemaining = theJsonString;
    aRemaining = p::removeLeadingWhitespace(aRemaining);

    const auto aTypeOpt = p::detectElementType(aRemaining);
    if (!aTypeOpt)
      return makeDocInfoError("Failed to determine element type");
    switch (*aTypeOpt) {
    case Type::NUL:
      if (const auto aNullLen = p::readNull(aRemaining).size(); aNullLen > 0) {
        aRemaining.remove_prefix(aNullLen);
        return {DocumentInfo::SingleNull(),
                theJsonString.size() - aRemaining.size()};
      } else
        return makeDocInfoError("Failed to read element: Expected null");
    case Type::BOOL:
      if (const auto [_, aBoolLen] = p::parseBool(aRemaining); aBoolLen > 0) {
        aRemaining.remove_prefix(aBoolLen);
        return {DocumentInfo::SingleBool(),
                theJsonString.size() - aRemaining.size()};
      } else
        return makeDocInfoError(
            "Failed to read element: Expected true or false");
    case Type::NUMBER:
      if (const auto aNumLen = p::readNumber(aRemaining).size(); aNumLen > 0) {
        aRemaining.remove_prefix(aNumLen);
        return {DocumentInfo::SingleNumber(),
                theJsonString.size() - aRemaining.size()};
      } else
        return makeDocInfoError(
            "Failed to read element: Expected valid number");
    case Type::STRING: {
      const std::string_view aStr = p::readString(aRemaining);
      if (const auto aStrLen = aStr.size(); aStrLen > 0) {
        aRemaining.remove_prefix(aStrLen);
        const auto aNumChars = p::template computeEncodedSize<DestEncodingTy>(
            p::stripQuotes(aStr));
        return {DocumentInfo::SingleString(aNumChars),
                theJsonString.size() - aRemaining.size()};
      } else
        return makeDocInfoError(
            "Failed to read element: Expected valid string");
    }
    case Type::ARRAY: {
      DocumentInfo aResult = DocumentInfo::EmptyArray();
      aRemaining.remove_prefix(SourceEncodingTy::encode('[').second);
      for (bool aIsFirst = true;; aIsFirst = false) {
        aRemaining = p::removeLeadingWhitespace(aRemaining);
        const auto [aChar, aCharWidth] =
            SourceEncodingTy::decodeFirst(aRemaining);
        if (aCharWidth <= 0)
          return makeDocInfoError("Failed to read array: Illegal first "
                                  "character or unexpected EOF");
        if (aChar == ']') {
          aRemaining.remove_prefix(aCharWidth);
          return {aResult, theJsonString.size() - aRemaining.size()};
        }
        if (!aIsFirst && aChar != ',')
          return makeDocInfoError("Failed to read array: Expected comma");
        if (aChar == ',')
          aRemaining.remove_prefix(aCharWidth);
        const auto [aSubDoc, aLen] = compute<SourceEncodingTy, DestEncodingTy>(aRemaining);
        if (!aSubDoc)
          return {aSubDoc, aLen}; // Propagate error
        aResult += aSubDoc;
        ++aResult.itsNumArrayEntries;
        aRemaining.remove_prefix(aLen);
      }
      return makeDocInfoError("UNREACHABLE");
    }
    case Type::OBJECT: {
      DocumentInfo aResult = DocumentInfo::EmptyObject();
      aRemaining.remove_prefix(SourceEncodingTy::encode('{').second);
      for (bool aIsFirst = true;; aIsFirst = false) {
        aRemaining = p::removeLeadingWhitespace(aRemaining);
        { // first char introspection
          const auto [aChar, aCharWidth] =
              SourceEncodingTy::decodeFirst(aRemaining);
          if (aCharWidth <= 0)
            return makeDocInfoError("Failed to read object: Illegal first "
                                    "character or unexpected EOF");
          if (aChar == '}') {
            aRemaining.remove_prefix(aCharWidth);
            return {aResult, theJsonString.size() - aRemaining.size()};
          }
          if (!aIsFirst && aChar != ',')
            return makeDocInfoError("Failed to read object: Expected comma");
          if (aChar == ',') {
            aRemaining.remove_prefix(aCharWidth);
            // need to strip whitespace - readString expects to start on '"'
            aRemaining = p::removeLeadingWhitespace(aRemaining);
          }
        }
        { // consume key
          const std::string_view aKey = p::readString(aRemaining);
          if (aKey.size() == 0)
            return makeDocInfoError(
                "Failed to read object: Could not read property key");
          ++aResult.itsNumStrings;
          aResult.itsNumChars += p::template computeEncodedSize<DestEncodingTy>(
              p::stripQuotes(aKey));
          aRemaining.remove_prefix(aKey.size());
        }
        { // consume colon
          aRemaining = p::removeLeadingWhitespace(aRemaining);
          const auto [aColon, aColonWidth] =
              SourceEncodingTy::decodeFirst(aRemaining);
          if (aColon != ':')
            return makeDocInfoError("Failed to read object: Expected colon");
          aRemaining.remove_prefix(aColonWidth);
        }
        const auto [aSubDoc, aLen] = compute<SourceEncodingTy, DestEncodingTy>(aRemaining);
        if (!aSubDoc)
          return {aSubDoc, aLen}; // Propagate error
        aResult += aSubDoc;
        ++aResult.itsNumObjectProperties;
        aRemaining.remove_prefix(aLen);
      }
      return makeDocInfoError("UNREACHABLE");
    }
    }
    return makeDocInfoError("UNREACHABLE");
  }

private:
  constexpr static std::pair<DocumentInfo, ssize_t>
  makeDocInfoError(const char *const theReason) {
    return {DocumentInfo::error(), -1};
  }
};
} // namespace cjson

#endif // CONSTEXPR_JSON_DOCUMENT_INFO_H
