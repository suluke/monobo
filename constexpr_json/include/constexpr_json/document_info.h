#ifndef CONSTEXPR_JSON_DOCUMENT_INFO_H
#define CONSTEXPR_JSON_DOCUMENT_INFO_H

#include "constexpr_json/error_codes.h"
#include "constexpr_json/impl/parsing_utils.h"

namespace cjson {
struct DocumentInfo {
  intptr_t itsNumNulls = 0;
  intptr_t itsNumBools = 0;
  intptr_t itsNumNumbers = 0;
  /// The number of chars (bytes) required to store all encoded string data.
  /// NOTE: '\n' e.g. takes 2 chars in the source json string but only 1 in
  /// the encoded string data.
  intptr_t itsNumChars = 0;
  intptr_t itsNumStrings = 0;
  intptr_t itsNumArrays = 0;
  intptr_t itsNumArrayEntries = 0;
  intptr_t itsNumObjects = 0;
  intptr_t itsNumObjectProperties = 0;

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
  constexpr static DocumentInfo SingleString(intptr_t theNumChars) {
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
      return (!(*this) && !theOther);
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

  using ResultTy = std::pair<DocumentInfo, intptr_t>;

  /**
   * @return .first is the DocInfo, .second is the number of read chars (i.e.
   * bytes, not code points)
   */
  template <typename SourceEncodingTy, typename DestEncodingTy,
            typename ErrorHandlingTy, intptr_t MaxRecursionDepth = 100>
  constexpr static typename ErrorHandlingTy::template ErrorOr<ResultTy>
  compute(const std::string_view theJsonString,
          const SourceEncodingTy theSrcEnc = SourceEncodingTy{},
          const DestEncodingTy theDestEnc = DestEncodingTy{},
          const intptr_t theRecursionDepth = 0) {
    // setup
    using P = parsing<SourceEncodingTy>;
    const P p{theSrcEnc};
    using Type = typename P::Type;
    std::string_view aRemaining = theJsonString;
#define CJSON_CURRENT_POSITION theJsonString.size() - aRemaining.size()
    aRemaining = p.removeLeadingWhitespace(aRemaining);

    // limit recursion
    if (MaxRecursionDepth >= 0 && theRecursionDepth > MaxRecursionDepth)
      return makeError<ErrorHandlingTy>(ErrorCode::MAX_DEPTH_EXCEEDED,
                                        CJSON_CURRENT_POSITION);

    const auto aTypeOpt = p.detectElementType(aRemaining);
    if (!aTypeOpt)
      return makeError<ErrorHandlingTy>(ErrorCode::TYPE_DEDUCTION_FAILED,
                                        CJSON_CURRENT_POSITION);
    switch (*aTypeOpt) {
    case Type::NUL:
      if (const auto aNullLen = p.readNull(aRemaining).size(); aNullLen > 0) {
        aRemaining.remove_prefix(aNullLen);
        return std::make_pair(DocumentInfo::SingleNull(),
                              CJSON_CURRENT_POSITION);
      } else
        return makeError<ErrorHandlingTy>(ErrorCode::NULL_READ_FAILED,
                                          CJSON_CURRENT_POSITION);
    case Type::BOOL:
      if (const auto aBoolLen = p.parseBool(aRemaining).second; aBoolLen > 0) {
        aRemaining.remove_prefix(aBoolLen);
        return std::make_pair(DocumentInfo::SingleBool(),
                              CJSON_CURRENT_POSITION);
      } else
        return makeError<ErrorHandlingTy>(ErrorCode::BOOL_READ_FAILED,
                                          CJSON_CURRENT_POSITION);
    case Type::NUMBER:
      if (const auto aNumLen = p.readNumber(aRemaining).size(); aNumLen > 0) {
        aRemaining.remove_prefix(aNumLen);
        return std::make_pair(DocumentInfo::SingleNumber(),
                              CJSON_CURRENT_POSITION);
      } else
        return makeError<ErrorHandlingTy>(ErrorCode::NUMBER_READ_FAILED,
                                          CJSON_CURRENT_POSITION);
    case Type::STRING: {
      const std::string_view aStr = p.readString(aRemaining);
      if (const auto aStrLen = aStr.size(); aStrLen > 0) {
        aRemaining.remove_prefix(aStrLen);
        const auto aNumChars = p.template computeEncodedSize<DestEncodingTy>(
            p.stripQuotes(aStr), theDestEnc);
        return std::make_pair(DocumentInfo::SingleString(aNumChars),
                              CJSON_CURRENT_POSITION);
      } else
        return makeError<ErrorHandlingTy>(ErrorCode::STRING_READ_FAILED,
                                          CJSON_CURRENT_POSITION);
    }
    case Type::ARRAY: {
      DocumentInfo aResult = DocumentInfo::EmptyArray();
      aRemaining.remove_prefix(theSrcEnc.encode('[').second);
      for (bool aIsFirst = true;; aIsFirst = false) {
        aRemaining = p.removeLeadingWhitespace(aRemaining);
        const auto [aChar, aCharWidth] = theDestEnc.decodeFirst(aRemaining);
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
                aRemaining, theSrcEnc, theDestEnc, theRecursionDepth + 1);
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
      aRemaining.remove_prefix(theSrcEnc.encode('{').second);
      for (bool aIsFirst = true;; aIsFirst = false) {
        aRemaining = p.removeLeadingWhitespace(aRemaining);
        { // first char introspection
          const auto [aChar, aCharWidth] = theSrcEnc.decodeFirst(aRemaining);
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
            aRemaining = p.removeLeadingWhitespace(aRemaining);
          }
        }
        { // consume key
          const std::string_view aKey = p.readString(aRemaining);
          if (aKey.size() == 0)
            return makeError<ErrorHandlingTy>(ErrorCode::OBJECT_KEY_READ_FAILED,
                                              CJSON_CURRENT_POSITION);
          ++aResult.itsNumStrings;
          aResult.itsNumChars += p.template computeEncodedSize<DestEncodingTy>(
              p.stripQuotes(aKey), theDestEnc);
          aRemaining.remove_prefix(aKey.size());
        }
        { // consume colon
          aRemaining = p.removeLeadingWhitespace(aRemaining);
          const auto [aColon, aColonWidth] = theSrcEnc.decodeFirst(aRemaining);
          if (aColon != ':')
            return makeError<ErrorHandlingTy>(ErrorCode::OBJECT_EXPECTED_COLON,
                                              CJSON_CURRENT_POSITION);
          aRemaining.remove_prefix(aColonWidth);
        }
        const auto aSubDocOrError =
            compute<SourceEncodingTy, DestEncodingTy, ErrorHandlingTy>(
                aRemaining, theSrcEnc, theDestEnc, theRecursionDepth + 1);
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

  template <typename JSON>
  static constexpr DocumentInfo read(const JSON &theJson) {
    DocumentInfo aResult;
    using Type = decltype(theJson.getType());
    switch (theJson.getType()) {
    case Type::NUL:
      aResult.itsNumNulls += 1;
      break;
    case Type::BOOL:
      aResult.itsNumBools += 1;
      break;
    case Type::NUMBER:
      aResult.itsNumNumbers += 1;
      break;
    case Type::STRING:
      aResult.itsNumChars += theJson.toString().size();
      aResult.itsNumStrings += 1;
      break;
    case Type::ARRAY:
      aResult.itsNumArrayEntries += theJson.toArray().size();
      aResult.itsNumArrays += 1;
      for (const auto &aEntry : theJson.toArray())
        aResult += read(aEntry);
      break;
    case Type::OBJECT:
      aResult.itsNumObjectProperties += theJson.toObject().size();
      aResult.itsNumStrings += theJson.toObject().size();
      aResult.itsNumObjects += 1;
      for (const auto &aKVPair : theJson.toObject()) {
        aResult.itsNumChars += aKVPair.first.size();
        aResult += read(aKVPair.second);
      }
      break;
    }
    return aResult;
  }

private:
  template <typename ErrorHandlingTy>
  constexpr static typename ErrorHandlingTy::template ErrorOr<ResultTy>
  makeError(const ErrorCode theCode, const intptr_t thePosition) {
    return ErrorHandlingTy::template makeError<ResultTy>(theCode, thePosition);
  }
};
} // namespace cjson

#endif // CONSTEXPR_JSON_DOCUMENT_INFO_H
