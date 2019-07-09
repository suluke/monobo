#ifndef CONSTEXPR_JSON_UTILS_PARSING_H
#define CONSTEXPR_JSON_UTILS_PARSING_H

#include <cctype>

namespace cjson {
template <typename EncodingTy> struct parsing {
private:
  using CharT = typename EncodingTy::CodePointTy;
  static auto decodeFirst(std::string_view theString) {
    return EncodingTy::decodeFirst(theString);
  }

public:
  constexpr std::pair<double, ssize_t> parseNumber(std::string_view theString) {
    return std::make_pair(0., -1);
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
      aDigits = {aDigits.data(), aDigits.size() + aCharWidth};
    }
  }
  static constexpr std::string_view readFraction(std::string_view theString) {
    return "";
  }
  static constexpr std::pair<int, ssize_t>
  parseExponent(std::string_view theString) {
    return std::make_pair(0, -1);
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