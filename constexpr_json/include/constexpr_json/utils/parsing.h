#ifndef CONSTEXPR_JSON_UTILS_PARSING_H
#define CONSTEXPR_JSON_UTILS_PARSING_H

#include <optional>
#include <tuple>

namespace cjson {
template <typename EncodingTy> struct parsing {
private:
  using CharT = typename EncodingTy::CodePointTy;
  constexpr static auto decodeFirst(std::string_view theString) {
    return EncodingTy::decodeFirst(theString);
  }

public:
  parsing() = delete;
  parsing(const parsing &) = delete;
  parsing(parsing &&) = delete;
  parsing &operator=(const parsing &) = delete;
  parsing &operator=(parsing &&) = delete;

  constexpr static bool isdigit(CharT theChar) {
    return '0' <= theChar && theChar <= '9';
  }
  constexpr static bool isxdigit(CharT theChar) {
    return isdigit(theChar) || ('A' <= theChar && theChar <= 'F') ||
           ('a' <= theChar && theChar <= 'f');
  }

  enum class Type { NUL, BOOL, NUMBER, STRING, ARRAY, OBJECT };

  constexpr static std::optional<Type>
  detectElementType(const std::string_view theJsonStr) {
    constexpr const std::optional<Type> aErrorResult = std::nullopt;
    const std::string_view aPreWS = readWhitespace(theJsonStr);
    std::string_view aRemaining = theJsonStr.substr(aPreWS.size());
    const auto [aFirstChar, aFirstCharWidth] = decodeFirst(aRemaining);
    if (aFirstCharWidth <= 0)
      // Expected *something*
      return aErrorResult;
    // Reduce digits to '0' to avoid some case labels
    const auto charZeroIfDigit = [](CharT aChar) -> CharT {
      return isdigit(aChar) ? '0' : aChar;
    };
    switch (charZeroIfDigit(aFirstChar)) {
    case 'n':
      return Type::NUL;
    case 't':
      [[fallthrough]];
    case 'f':
      return Type::BOOL;
    case '0':
      return Type::NUMBER;
    case '"':
      return Type::STRING;
    case '[':
      return Type::ARRAY;
    case '{':
      return Type::OBJECT;
    }
    return aErrorResult;
  }

  template <typename ElementTy>
  constexpr static std::pair<ElementTy, ssize_t>
  parseElement(const std::string_view theString) {
    constexpr const std::pair<ElementTy, ssize_t> aErrorResult =
        std::make_pair(ElementTy::null(), -1);
    // Step 1: Consume whitespace
    const std::string_view aPreWS = readWhitespace(theString);
    std::string_view aRemaining = theString.substr(aPreWS.size());
    ElementTy aElement;
    // Step 2: Detect type of element
    const std::optional<Type> aTypeMaybe = detectElementType(aRemaining);
    if (!aTypeMaybe)
      return aErrorResult;
    Type aElmTy = *aTypeMaybe;
    // Step 3: Parse element
    switch (aElmTy) {
    case Type::NUL: {
      const ssize_t aNullLen = parseNull(aRemaining);
      if (aNullLen <= 0)
        return aErrorResult;
      aElement.setNull();
      aRemaining.remove_prefix(aNullLen);
      break;
    }
    case Type::BOOL: {
      const auto [aBool, aBoolLen] = parseBool(aRemaining);
      if (aBoolLen <= 0)
        return aErrorResult;
      aElement.setBool(aBool);
      aRemaining.remove_prefix(aBoolLen);
      break;
    }
    case Type::NUMBER: {
      const auto [aNum, aNumLen] = parseNumber(aRemaining);
      if (aNumLen <= 0)
        return aErrorResult;
      aElement.setNumber(aNum);
      aRemaining.remove_prefix(aNumLen);
      break;
    }
    case Type::STRING: {
      std::string_view aStr = readString(aRemaining);
      if (aStr.empty())
        return aErrorResult;
      aRemaining.remove_prefix(aStr.size());
      // Remove quotation marks surrounding string
      const auto [aQuot, aQuotWidth] = decodeFirst(aStr);
      aStr.remove_prefix(aQuotWidth);
      aStr.remove_suffix(aQuotWidth);
      aElement.setString(aStr);
      break;
    }
    case Type::ARRAY: {
      const auto [aArray, aArrayLen] = parseArray<ElementTy>(aRemaining);
      if (aArrayLen <= 0)
        return aErrorResult;
      aRemaining.remove_prefix(aArrayLen);
      aElement = aArray;
      break;
    }
    case Type::OBJECT: {
      const auto [aObject, aObjectLen] = parseObject<ElementTy>(aRemaining);
      if (aObjectLen <= 0)
        return aErrorResult;
      aRemaining.remove_prefix(aObjectLen);
      aElement = aObject;
      break;
    }
    default:
      return aErrorResult;
    }
    const std::string_view aPostWS = readWhitespace(aRemaining);
    aRemaining.remove_prefix(aPostWS.size());
    return std::make_pair(aElement, theString.size() - aRemaining.size());
  }

  template <typename ElementTy>
  constexpr static std::pair<ElementTy, ssize_t>
  parseObject(const std::string_view theString) {
    constexpr const std::pair<ElementTy, ssize_t> aErrorResult =
        std::make_pair(ElementTy::null(), -1);
    const auto [aFirstChar, aFirstCharWidth] = decodeFirst(theString);
    if (aFirstCharWidth <= 0)
      // Failed to decode first char
      return aErrorResult;
    if (aFirstChar != '{')
      return aErrorResult;
    std::string_view aRemaining = theString.substr(aFirstCharWidth);
    const std::string_view aWS = readWhitespace(aRemaining);
    aRemaining.remove_prefix(aWS.size());
    bool aNeedsMoreProps = false;
    ElementTy aElement;
    aElement.setObject();
    for (;;) {
      const auto [aChar, aCharWidth] = decodeFirst(aRemaining);
      if (aCharWidth <= 0)
        return aErrorResult;
      if (!aNeedsMoreProps && aChar == '}') {
        aRemaining.remove_prefix(aCharWidth);
        break;
      }
      const std::string_view aLeadWs = readWhitespace(aRemaining);
      aRemaining.remove_prefix(aLeadWs.size());
      // Read key
      std::string_view aKey = readString(aRemaining);
      if (aKey.size() <= 0)
        return aErrorResult;
      aRemaining.remove_prefix(aKey.size());
      const std::string_view aTrailWs = readWhitespace(aRemaining);
      aRemaining.remove_prefix(aTrailWs.size());
      // Read colon
      const auto [aColon, aColonWidth] = decodeFirst(aRemaining);
      if (aColonWidth <= 0)
        return aErrorResult;
      if (aColon != ':')
        return aErrorResult;
      // Read element
      aRemaining.remove_prefix(aColonWidth);
      const auto [aElm, aElmLength] = parseElement<ElementTy>(aRemaining);
      if (aElmLength <= 0)
        return aErrorResult;
      // Remove quotation marks surrounding key
      const auto [aQuot, aQuotWidth] = decodeFirst(aKey);
      aKey.remove_prefix(aQuotWidth);
      aKey.remove_suffix(aQuotWidth);
      aElement.addObjectProperty(aKey, aElm);
      aRemaining.remove_prefix(aElmLength);
      // Look for ','
      const auto [aPeekChar, aPeekCharWidth] = decodeFirst(aRemaining);
      if (aPeekCharWidth <= 0)
        return aErrorResult;
      if (aPeekChar == ',') {
        aNeedsMoreProps = true;
        aRemaining.remove_prefix(aPeekCharWidth);
      } else {
        aNeedsMoreProps = false;
      }
    }
    return std::make_pair(aElement, theString.size() - aRemaining.size());
  }

  template <typename ElementTy>
  constexpr static std::pair<ElementTy, ssize_t>
  parseArray(const std::string_view theString) {
    constexpr const std::pair<ElementTy, ssize_t> aErrorResult =
        std::make_pair(ElementTy::null(), -1);
    const auto [aFirstChar, aFirstCharWidth] = decodeFirst(theString);
    if (aFirstCharWidth <= 0)
      // Failed to decode first char
      return aErrorResult;
    if (aFirstChar != '[')
      return aErrorResult;
    std::string_view aRemaining = theString.substr(aFirstCharWidth);
    const std::string_view aWS = readWhitespace(aRemaining);
    aRemaining.remove_prefix(aWS.size());
    bool aNeedsElement = false;
    ElementTy aElement;
    aElement.setArray();
    for (;;) {
      const auto [aChar, aCharWidth] = decodeFirst(aRemaining);
      if (aCharWidth <= 0)
        return aErrorResult;
      if (!aNeedsElement && aChar == ']') {
        aRemaining.remove_prefix(aCharWidth);
        break;
      }
      const auto [aItem, aItemLength] = parseElement<ElementTy>(aRemaining);
      if (aItemLength <= 0)
        return aErrorResult;
      aElement.addArrayEntry(aItem);
      aRemaining.remove_prefix(aItemLength);
      const auto [aPeekChar, aPeekCharWidth] = decodeFirst(aRemaining);
      if (aPeekCharWidth <= 0)
        return aErrorResult;
      if (aPeekChar == ',') {
        aNeedsElement = true;
        aRemaining.remove_prefix(aPeekCharWidth);
      } else {
        aNeedsElement = false;
      }
    }
    return std::make_pair(aElement, theString.size() - aRemaining.size());
  }

  constexpr static std::string_view
  readString(const std::string_view theString) {
    constexpr const std::string_view aErrorResult;
    const auto [aFirstChar, aFirstCharWidth] = decodeFirst(theString);
    if (aFirstCharWidth <= 0)
      // Failed to decode first char
      return aErrorResult;
    if (aFirstChar != '"')
      // Expected '"', got something else
      return aErrorResult;
    std::string_view aRemaining = theString.substr(aFirstCharWidth);
    for (;;) {
      const auto [aChar, aCharWidth] = decodeFirst(aRemaining);
      if (aCharWidth <= 0)
        // Failed to decode first char
        return aErrorResult;
      if (aChar == '"') {
        // Found closing '"'
        aRemaining.remove_prefix(aCharWidth);
        return {theString.data(), theString.size() - aRemaining.size()};
      } else if (aChar == '\\') {
        const auto [aEscape, aEscapeWidth] = parseEscape(aRemaining);
        std::ignore = aEscape;
        aRemaining.remove_prefix(aEscapeWidth);
      } else if (0x20 > aChar || aChar > 0x10ffff) {
        // Not a valid JSON char
        return aErrorResult;
      } else {
        aRemaining.remove_prefix(aCharWidth);
      }
    }
  }

  constexpr static std::pair<CharT, ssize_t>
  parseEscape(const std::string_view theString) {
    constexpr const std::pair<CharT, ssize_t> aErrorResult =
        std::make_pair(0, -1);
    const auto [aFirstChar, aFirstCharWidth] = decodeFirst(theString);
    if (aFirstCharWidth <= 0)
      // Failed to decode first char
      return aErrorResult;
    if (aFirstChar != '\\')
      // Expected '\\', got something else
      return aErrorResult;
    std::string_view aRemaining = theString.substr(aFirstCharWidth);
    const auto [aSecondChar, aSecondCharWidth] = decodeFirst(aRemaining);
    if (aSecondCharWidth <= 0)
      // Failed to decode second char
      return aErrorResult;
    const auto hexToNibble = [](CharT aHexChar) {
      if ('0' <= aHexChar && aHexChar <= '9')
        return aHexChar - '0';
      if ('A' <= aHexChar && aHexChar <= 'F')
        return aHexChar - 'A';
      if ('a' <= aHexChar && aHexChar <= 'f')
        return aHexChar - 'a';
      return CharT{}; // FIXME throw or terminate
    };
    aRemaining.remove_prefix(aSecondCharWidth);
    CharT aDecoded = 0;
    switch (aSecondChar) {
    case '"':
      aDecoded = '"';
      break;
    case '\\':
      aDecoded = '\\';
      break;
    case '/':
      aDecoded = '/';
      break;
    case 'b':
      aDecoded = '\b';
      break;
    case 'f':
      aDecoded = '\f';
      break;
    case 'n':
      aDecoded = '\n';
      break;
    case 'r':
      aDecoded = '\r';
      break;
    case 't':
      aDecoded = '\t';
      break;
    case 'u': {
      for (int i = 0; i < 4; ++i) {
        const auto [aChar, aCharWidth] = decodeFirst(aRemaining);
        if (aCharWidth <= 0)
          // Failed to decode char
          return aErrorResult;
        if (!isxdigit(aChar))
          // Not a valid hex char
          return aErrorResult;
        aRemaining.remove_prefix(aCharWidth);
        aDecoded <<= 4;
        aDecoded |= hexToNibble(aChar);
      }
      break;
    }
    default:
      return aErrorResult;
    }
    return {aDecoded, theString.size() - aRemaining.size()};
  }

  constexpr static std::pair<bool, ssize_t>
  parseBool(const std::string_view theString) {
    constexpr const std::pair<bool, ssize_t> aErrorResult =
        std::make_pair(false, -1);
    const auto [aFirstChar, aFirstCharWidth] = decodeFirst(theString);
    if (aFirstCharWidth <= 0)
      // Failed to decode first char
      return aErrorResult;
    if (aFirstChar != 't' && aFirstChar != 'f')
      // Expected 't' or 'f', got something else
      return aErrorResult;
    const bool aExpectedVal = aFirstChar == 't';
    const std::string_view aCmpStr = aExpectedVal ? "true" : "false";
    std::string_view aRemaining = theString;
    for (const char aExpected : aCmpStr) {
      const auto [aChar, aCharWidth] = decodeFirst(aRemaining);
      if (aCharWidth <= 0)
        // failed to decode char
        return aErrorResult;
      if (static_cast<CharT>(aExpected) != aChar)
        return aErrorResult;
      aRemaining.remove_prefix(aCharWidth);
    }
    return std::make_pair(aExpectedVal, theString.size() - aRemaining.size());
  }

  constexpr static ssize_t parseNull(const std::string_view theString) {
    constexpr const ssize_t aErrorResult = -1;
    const std::string_view aCmpStr = "null";
    std::string_view aRemaining = theString;
    for (const char aExpected : aCmpStr) {
      const auto [aChar, aCharWidth] = decodeFirst(aRemaining);
      if (aCharWidth <= 0)
        // failed to decode char
        return aErrorResult;
      if (static_cast<CharT>(aExpected) != aChar)
        return aErrorResult;
      aRemaining.remove_prefix(aCharWidth);
    }
    return theString.size() - aRemaining.size();
  }

  constexpr static std::pair<double, ssize_t>
  parseNumber(const std::string_view theString) {
    constexpr const std::pair<double, ssize_t> aErrorResult =
        std::make_pair(0., -1);
    // Step 1: Read int
    const auto [aInt, aIntLength] = parseInteger(theString);
    if (aIntLength <= 0)
      // the integer part is at least one character wide
      return aErrorResult;
    std::string_view aRemaining = theString.substr(aIntLength);
    // Step 2: Read fraction
    std::string_view aFractionStr = readFraction(aRemaining);
    double aFrac = 0.;
    if (!aFractionStr.empty()) {
      aRemaining.remove_prefix(aFractionStr.size());
      // Drop leading '.'
      aFractionStr.remove_prefix(decodeFirst(aFractionStr).second);
      // FIXME not a precise way to parse digits
      double aDenom = 10.;
      for (;;) {
        const auto [aChar, aCharWidth] = decodeFirst(aFractionStr);
        // Unneeded safety check
        if (aCharWidth <= 0)
          // Failed to decode a digit
          return aErrorResult;
        aFrac += (aChar - '0') / aDenom;
        aDenom *= 10.;
        if (aCharWidth > aFractionStr.size())
          // how did we run out of characters?
          return aErrorResult;
        aFractionStr.remove_prefix(aCharWidth);
        if (aFractionStr.empty())
          break;
      }
    }
    // Step 3: Read exponent
    const auto [aExp, aExpLength] = parseExponent(aRemaining);
    if (aExpLength < 0)
      return aErrorResult;
    aRemaining.remove_prefix(aExpLength);
    // Step 4: Combine int, frac and exp
    double aResult = aInt + (aInt >= 0 ? aFrac : -aFrac);
    if (aExp == 0) {
      aResult = 1.;
    } else if (aExp > 0) {
      for (int i = 1; i < aExp; ++i)
        aResult *= 10.;
    } else {
      aResult = 1. / aResult;
      for (int i = 0; i < -aExp; ++i)
        aResult /= 10.;
    }
    return std::make_pair(aResult, theString.size() - aRemaining.size());
  }

  /// The return type double is chosen for encoding negative zero and because
  /// it has the same integer precision as the parse target type (which is
  /// also double)
  /// @return the parsed integer and the number of bytes consumed while
  /// parsing.
  static constexpr std::pair<double, ssize_t>
  parseInteger(const std::string_view theString) {
    constexpr const auto aErrorResult = std::make_pair(0., -1);
    if (theString.empty())
      // Cannot parse integer from empty string
      return aErrorResult;
    std::string_view aRemaining = theString;
    // Step 1: Test for sign
    int sign = 1;
    {
      const auto [aChar, aCharWith] = decodeFirst(aRemaining);
      if (aCharWith <= 0)
        // Cannot decode first char
        return aErrorResult;
      if (aChar == '-') {
        sign = -1;
        aRemaining.remove_prefix(aCharWith);
      }
    }
    // Step 2: Test if first digit is '0'
    {
      const auto [aChar, aCharWith] = decodeFirst(aRemaining);
      if (aCharWith <= 0)
        // Cannot decode first char
        return aErrorResult;
      if (aChar == '0')
        return std::make_pair(sign * 0.,
                              theString.size() - aRemaining.size() + aCharWith);
    }
    // Step 3: Read and parse digits
    std::string_view aDigits = readDigits(aRemaining);
    if (aDigits.empty())
      return aErrorResult;
    int aParsedInt = 0;
    size_t aParsedChars = theString.size() - aRemaining.size();
    for (;;) {
      const auto [aChar, aCharWidth] = decodeFirst(aDigits);
      if (aCharWidth <= 0)
        // Failed to decode a character
        return aErrorResult;
      if (aCharWidth > aDigits.size())
        // We ran out of remaining characters(?!)
        return aErrorResult;
      aDigits.remove_prefix(aCharWidth);
      int aDigit = aChar - '0';
      aParsedInt *= 10;
      aParsedInt += aDigit;
      aParsedChars += aCharWidth;
      // No more digits to parse, we're done
      if (aDigits.empty())
        return std::make_pair(sign * aParsedInt, aParsedChars);
    }
  }

  static constexpr std::string_view
  readDigits(const std::string_view theString) {
    std::string_view aDigits(theString.data(), 0);
    std::string_view aRemaining = theString;
    for (;;) {
      const auto [aChar, aCharWidth] = decodeFirst(aRemaining);
      if (aCharWidth <= 0)
        // Failed to decode a character. But we don't do error handling here
        return aDigits;
      if (!isdigit(aChar))
        // Found a non-digit character
        return aDigits;
      if (aCharWidth > aRemaining.size())
        // We ran out of remaining characters(?!)
        return aDigits;
      aRemaining.remove_prefix(aCharWidth);
      aDigits = {theString.data(), aDigits.size() + aCharWidth};
    }
  }

  static constexpr std::string_view
  readFraction(const std::string_view theString) {
    if (theString.empty())
      return "";
    const auto [aChar, aCharWidth] = decodeFirst(theString);
    if (aCharWidth <= 0)
      // failed to decode first char
      return "";
    if (aChar != '.')
      // expected '.', got something else
      return "";
    std::string_view aDigits = readDigits(theString.substr(aCharWidth));
    if (aDigits.empty())
      // expected at least one digit
      return "";
    return theString.substr(0, aDigits.size() + aCharWidth);
  }

  /// Expects a leading 'e' or 'E', then parses a (signed) int where leading
  /// '0' are allowed as per the JSON spec
  static constexpr std::pair<int, ssize_t>
  parseExponent(const std::string_view theString) {
    if (theString.empty())
      return std::make_pair(1, 0);
    constexpr const auto aErrorResult = std::make_pair(0, -1);

    // Step 1: Check for expected 'e'/'E'
    const auto [aChar, aCharWidth] = decodeFirst(theString);
    if (aCharWidth <= 0)
      // failed to decode first char
      return aErrorResult;
    if (aChar != 'e' && aChar != 'E')
      // expected 'e'/'E', got something else
      return std::make_pair(1, 0);
    std::string_view aRemaining = theString.substr(aCharWidth);

    // Step 2: Check for a sign char
    const auto [aSignChar, aSignWidth] = decodeFirst(aRemaining);
    int aSign = 1;
    if (aSignChar == '-' || aSignChar == '+') {
      if (aSignChar == '-')
        aSign = -1;
      aRemaining.remove_prefix(aSignWidth);
    }

    // Step 3: Read and parse exponent value
    std::string_view aExponentStr = readDigits(aRemaining);
    if (aExponentStr.empty())
      // Expected digits, got empty string
      return aErrorResult;
    aRemaining.remove_prefix(aExponentStr.size());
    int aExp = 0;
    for (;;) {
      const auto [aChar, aCharWidth] = decodeFirst(aExponentStr);
      // Unneeded safety check
      if (aCharWidth <= 0)
        // Failed to decode a digit
        return aErrorResult;
      aExp *= 10;
      aExp += (aChar - '0');
      if (aCharWidth > aExponentStr.size())
        // how did we run out of characters?
        return aErrorResult;
      aExponentStr.remove_prefix(aCharWidth);
      if (aExponentStr.empty())
        return std::make_pair(aSign * aExp,
                              theString.size() - aRemaining.size());
    }
  }

  static constexpr std::string_view
  readWhitespace(const std::string_view theString) {
    std::string_view aWhitespace(theString.data(), 0);
    std::string_view aRemaining(theString);
    for (;;) {
      const auto [aChar, aCharWidth] = decodeFirst(aRemaining);
      if (aCharWidth <= 0)
        // Failed to decode a character. But we don't do error handling here
        return aWhitespace;
      if (aChar != 0x20 && aChar != 0xd && aChar != 0xa && aChar != 0x9)
        // Found a non-whitespace character
        return aWhitespace;
      if (aCharWidth > aRemaining.size())
        // We ran out of remaining characters(?!)
        return aWhitespace;
      aRemaining.remove_prefix(aCharWidth);
      aWhitespace = {aWhitespace.data(), aWhitespace.size() + aCharWidth};
    }
  }
};
} // namespace cjson
#endif // CONSTEXPR_JSON_UTILS_PARSING_H
