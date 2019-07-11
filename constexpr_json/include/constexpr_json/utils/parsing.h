#ifndef CONSTEXPR_JSON_UTILS_PARSING_H
#define CONSTEXPR_JSON_UTILS_PARSING_H

#include <cctype>
#include <tuple>

namespace cjson {
template <typename EncodingTy> struct parsing {
private:
  using CharT = typename EncodingTy::CodePointTy;
  static auto decodeFirst(std::string_view theString) {
    return EncodingTy::decodeFirst(theString);
  }

public:
  template <typename ElementTy>
  constexpr static ssize_t parseElement(std::string_view theString,
                                        ElementTy &theElement) {
    constexpr const ssize_t aErrorResult = -1;
    // Step 1: Consume whitespace
    const std::string_view aPreWS = readWhitespace(theString);
    std::string_view aRemaining = theString.substr(aPreWS.size());
    const auto [aFirstChar, aFirstCharWidth] = decodeFirst(aRemaining);
    if (aFirstCharWidth <= 0)
      // Expected *something*
      return aErrorResult;
    switch (aFirstChar) {
    case 't':
      [[fallthrough]];
    case 'f': {
      const auto [aBool, aBoolLen] = parseBool(aRemaining);
      if (aBoolLen <= 0)
        return aErrorResult;
      theElement.setBool(aBool);
      aRemaining.remove_prefix(aBoolLen);
      break;
    }
    case 'n': {
      const ssize_t aNullLen = parseNull(aRemaining);
      if (aNullLen <= 0)
        return aErrorResult;
      theElement.setNull();
      aRemaining.remove_prefix(aNullLen);
      break;
    }
    case '"': {
      // TODO
      break;
    }
    case '[': {
      // TODO
      break;
    }
    case '{': {
      // TODO
      break;
    }
    case '0':
      [[fallthrough]];
    case '1':
      [[fallthrough]];
    case '2':
      [[fallthrough]];
    case '3':
      [[fallthrough]];
    case '4':
      [[fallthrough]];
    case '5':
      [[fallthrough]];
    case '6':
      [[fallthrough]];
    case '7':
      [[fallthrough]];
    case '8':
      [[fallthrough]];
    case '9': {
      const auto [aNum, aNumLen] = parseNumber(aRemaining);
      if (aNumLen <= 0)
        return aErrorResult;
      theElement.setNumber(aNum);
      aRemaining.remove_prefix(aNumLen);
      break;
    }
    default:
      return aErrorResult;
    }
    const std::string_view aPostWS = readWhitespace(aRemaining);
    aRemaining.remove_prefix(aPostWS.size());
    return theString.size() - aRemaining.size();
  }

  constexpr static std::pair<bool, ssize_t>
  parseBool(std::string_view theString) {
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

  constexpr static ssize_t parseNull(std::string_view theString) {
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
  parseNumber(std::string_view theString) {
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

  /// @return the parsed integer and its length in bytes. The return type double
  /// is chosen for encoding negative zero
  static constexpr std::pair<double, ssize_t>
  parseInteger(std::string_view theString) {
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

  static constexpr std::string_view readDigits(std::string_view theString) {
    std::string_view aDigits(theString.data(), 0);
    std::string_view aRemaining = theString;
    for (;;) {
      const auto [aChar, aCharWidth] = decodeFirst(aRemaining);
      if (aCharWidth <= 0)
        // Failed to decode a character. But we don't do error handling here
        return aDigits;
      if (!std::isdigit(aChar))
        // Found a non-digit character
        return aDigits;
      if (aCharWidth > aRemaining.size())
        // We ran out of remaining characters(?!)
        return aDigits;
      aRemaining.remove_prefix(aCharWidth);
      aDigits = {theString.data(), aDigits.size() + aCharWidth};
    }
  }

  static constexpr std::string_view readFraction(std::string_view theString) {
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

  static constexpr std::pair<int, ssize_t>
  parseExponent(std::string_view theString) {
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

  static constexpr std::string_view readWhitespace(std::string_view theString) {
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