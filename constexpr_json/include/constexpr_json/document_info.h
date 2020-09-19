#ifndef CONSTEXPR_JSON_DOCUMENT_INFO_H
#define CONSTEXPR_JSON_DOCUMENT_INFO_H

#include "constexpr_json/error_codes.h"
#include "constexpr_json/utils/parsing.h"
#include <stdexcept>

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

  constexpr static DocumentInfo Invalid() {
    return {-1, -1, -1, -1, -1, -1, -1, -1, -1};
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
    if (!(*this) || !theOther)
      return Invalid();
    DocumentInfo aDI = *this;
    aDI += theOther;
    return aDI;
  }
  constexpr DocumentInfo &operator+=(const DocumentInfo &theOther) {
    if (!theOther)
      *this = Invalid();
    if (!(*this))
      return *this;
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
    // Falsy DocInfos are always considered to be equal
    if (!(*this) || !theOther)
      return (!(this) && !theOther);
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

  using ResultTy = std::pair<DocumentInfo, ssize_t>;

  /**
   * @return .first is the DocInfo, .second is the number of read chars (i.e.
   * bytes, not code points)
   */
  template <typename SourceEncodingTy, typename DestEncodingTy,
            typename ErrorHandlingTy>
  constexpr static typename ErrorHandlingTy::ErrorOr<ResultTy>
  compute(const std::string_view theJsonString) {
    // setup
    using p = parsing<SourceEncodingTy>;
    using Type = typename p::Type;
    std::string_view aRemaining = theJsonString;
#define CJSON_CURRENT_POSITION theJsonString.size() - aRemaining.size()
    aRemaining = p::removeLeadingWhitespace(aRemaining);

    const auto aTypeOpt = p::detectElementType(aRemaining);
    if (!aTypeOpt)
      return makeError<ErrorHandlingTy>(ErrorCode::TYPE_DEDUCTION_FAILED,
                                        CJSON_CURRENT_POSITION);
    switch (*aTypeOpt) {
    case Type::NUL:
      if (const auto aNullLen = p::readNull(aRemaining).size(); aNullLen > 0) {
        aRemaining.remove_prefix(aNullLen);
        return std::make_pair(DocumentInfo::SingleNull(),
                              CJSON_CURRENT_POSITION);
      } else
        return makeError<ErrorHandlingTy>(ErrorCode::NULL_READ_FAILED,
                                          CJSON_CURRENT_POSITION);
    case Type::BOOL:
      if (const auto [_, aBoolLen] = p::parseBool(aRemaining); aBoolLen > 0) {
        aRemaining.remove_prefix(aBoolLen);
        return std::make_pair(DocumentInfo::SingleBool(),
                              CJSON_CURRENT_POSITION);
      } else
        return makeError<ErrorHandlingTy>(ErrorCode::BOOL_READ_FAILED,
                                          CJSON_CURRENT_POSITION);
    case Type::NUMBER:
      if (const auto aNumLen = p::readNumber(aRemaining).size(); aNumLen > 0) {
        aRemaining.remove_prefix(aNumLen);
        return std::make_pair(DocumentInfo::SingleNumber(),
                              CJSON_CURRENT_POSITION);
      } else
        return makeError<ErrorHandlingTy>(ErrorCode::NUMBER_READ_FAILED,
                                          CJSON_CURRENT_POSITION);
    case Type::STRING: {
      const std::string_view aStr = p::readString(aRemaining);
      if (const auto aStrLen = aStr.size(); aStrLen > 0) {
        aRemaining.remove_prefix(aStrLen);
        const auto aNumChars = p::template computeEncodedSize<DestEncodingTy>(
            p::stripQuotes(aStr));
        return std::make_pair(DocumentInfo::SingleString(aNumChars),
                              CJSON_CURRENT_POSITION);
      } else
        return makeError<ErrorHandlingTy>(ErrorCode::STRING_READ_FAILED,
                                          CJSON_CURRENT_POSITION);
    }
    case Type::ARRAY: {
      DocumentInfo aResult = DocumentInfo::EmptyArray();
      aRemaining.remove_prefix(SourceEncodingTy::encode('[').second);
      for (bool aIsFirst = true;; aIsFirst = false) {
        aRemaining = p::removeLeadingWhitespace(aRemaining);
        const auto [aChar, aCharWidth] =
            SourceEncodingTy::decodeFirst(aRemaining);
        if (aCharWidth <= 0)
          return makeError<ErrorHandlingTy>(ErrorCode::ARRAY_UNEXPECTED_TOKEN,
                                            CJSON_CURRENT_POSITION);
        if (aChar == ']') {
          aRemaining.remove_prefix(aCharWidth);
          return std::make_pair(aResult, CJSON_CURRENT_POSITION);
        }
        if (!aIsFirst && aChar != ',')
          return makeError<ErrorHandlingTy>(ErrorCode::ARRAY_EXPECTED_COMMA,
                                            CJSON_CURRENT_POSITION);
        if (aChar == ',')
          aRemaining.remove_prefix(aCharWidth);
        const auto aSubDocOrError =
            compute<SourceEncodingTy, DestEncodingTy, ErrorHandlingTy>(
                aRemaining);
        if (ErrorHandlingTy::isError(aSubDocOrError))
          return ErrorHandlingTy::template convertError<ResultTy>(
              aSubDocOrError, CJSON_CURRENT_POSITION); // Propagate error
        const auto [aSubDoc, aLen] = ErrorHandlingTy::unwrap(aSubDocOrError);
        aResult += aSubDoc;
        ++aResult.itsNumArrayEntries;
        aRemaining.remove_prefix(aLen);
      }
      return makeError<ErrorHandlingTy>(ErrorCode::UNREACHABLE, -1);
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
            return makeError<ErrorHandlingTy>(
                ErrorCode::OBJECT_UNEXPECTED_TOKEN, CJSON_CURRENT_POSITION);
          if (aChar == '}') {
            aRemaining.remove_prefix(aCharWidth);
            return std::make_pair(aResult, CJSON_CURRENT_POSITION);
          }
          if (!aIsFirst && aChar != ',')
            return makeError<ErrorHandlingTy>(ErrorCode::OBJECT_EXPECTED_COMMA,
                                              CJSON_CURRENT_POSITION);
          if (aChar == ',') {
            aRemaining.remove_prefix(aCharWidth);
            // need to strip whitespace - readString expects to start on '"'
            aRemaining = p::removeLeadingWhitespace(aRemaining);
          }
        }
        { // consume key
          const std::string_view aKey = p::readString(aRemaining);
          if (aKey.size() == 0)
            return makeError<ErrorHandlingTy>(ErrorCode::OBJECT_KEY_READ_FAILED,
                                              CJSON_CURRENT_POSITION);
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
            return makeError<ErrorHandlingTy>(ErrorCode::OBJECT_EXPECTED_COLON,
                                              CJSON_CURRENT_POSITION);
          aRemaining.remove_prefix(aColonWidth);
        }
        const auto aSubDocOrError =
            compute<SourceEncodingTy, DestEncodingTy, ErrorHandlingTy>(
                aRemaining);
        if (ErrorHandlingTy::isError(aSubDocOrError))
          return ErrorHandlingTy::template convertError<ResultTy>(
              aSubDocOrError, CJSON_CURRENT_POSITION); // Propagate error
        const auto [aSubDoc, aLen] = ErrorHandlingTy::unwrap(aSubDocOrError);
        aResult += aSubDoc;
        ++aResult.itsNumObjectProperties;
        aRemaining.remove_prefix(aLen);
      }
      return makeError<ErrorHandlingTy>(ErrorCode::UNREACHABLE,
                                        CJSON_CURRENT_POSITION);
    }
    }
    return makeError<ErrorHandlingTy>(ErrorCode::UNREACHABLE,
                                      CJSON_CURRENT_POSITION);
#undef CJSON_CURRENT_POSITION
  }

private:
  template <typename ErrorHandlingTy>
  constexpr static typename ErrorHandlingTy::ErrorOr<ResultTy>
  makeError(const ErrorCode theCode, const intptr_t thePosition) {
    return ErrorHandlingTy::template makeError<ResultTy>(theCode, thePosition);
  }
};
} // namespace cjson

#endif // CONSTEXPR_JSON_DOCUMENT_INFO_H
