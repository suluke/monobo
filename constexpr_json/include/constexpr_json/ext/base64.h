#ifndef CONSTEXPR_JSON_EXT_BASE64_H
#define CONSTEXPR_JSON_EXT_BASE64_H

#include <array>
#include <cstddef>
#include <cstdint>
#include <string_view>
#include <utility>

namespace cjson {
/// Implementation of Base64 encoding
//
// This is of course a binary (bytes-to-bytes) encoding as opposed to our
// regular text (bytes-to-codepoint/codepoint-to-bytes) encodings. But
// with a little glue in between we should be able to combine the two
// domains transparently into e.g. a single utf8-atop-base64 text encoding
struct Base64 {
  struct CodePointTy : public std::array<char, 3> {
    uint_least8_t size;
  };
  static constexpr size_t MAX_BYTES = 4;

  /// @param theSextet value between 0 and (including) 63
  /// @return the base64 character corresponding to the input sextet, '\0' if
  /// input is not a valid sextet.
  static constexpr char base64Enc(unsigned char theSextet) {
    if (theSextet <= 25)
      return theSextet + 'A';
    else if (theSextet <= 51)
      return theSextet - 26 + 'a';
    else if (theSextet <= 61)
      return theSextet - 52 + '0';
    else if (theSextet == 62)
      return '+';
    else if (theSextet == 63)
      return '/';
    return '\0';
  }

  /// @return The binary value corresponding to the input base64 character, -1
  /// if invalid.
  constexpr static int32_t base64Dec(const char theB64) {
    if ('A' <= theB64 && theB64 <= 'Z')
      return theB64 - 'A';
    else if ('a' <= theB64 && theB64 <= 'z')
      return theB64 - 'a' + 26;
    else if ('0' <= theB64 && theB64 <= '9')
      return theB64 - '0' + 52;
    else if (theB64 == '+')
      return 62;
    else if (theB64 == '/')
      return 63;
    else if (theB64 == '=')
      return 0;
    return -1;
  }

  constexpr std::pair<CodePointTy, size_t>
  decodeFirst(std::string_view theString) const noexcept {
    auto aRes = std::make_pair<CodePointTy, size_t>({}, 0);
    if (theString.size() < 4)
      return aRes;
    uint32_t aBitbag{0};
    constexpr auto addBits = [](const char theB64, uint32_t &theBitbag) {
      theBitbag <<= 6;
      const auto aBin = base64Dec(theB64);
      if (aBin < 0)
        return false;
      theBitbag |= aBin;
      return true;
    };
    int aPaddings = 0;
    for (int i = 0; i < 4; ++i) {
      if (!addBits(theString[i], aBitbag))
        return aRes;
      aPaddings += ('=' == theString[i]);
    }

    CodePointTy &aBytes = aRes.first;
    aBytes[0] = (aBitbag >> 16) & 0xff;
    aBytes[1] = (aBitbag >> 8) & 0xff;
    aBytes[2] = (aBitbag >> 0) & 0xff;
    aBytes.size = 3 - aPaddings;
    aRes.second = 4;
    return aRes;
  }

  constexpr std::pair<std::array<char, MAX_BYTES>, size_t>
  encode(CodePointTy theCodePoint) const noexcept {
    auto aRes = std::make_pair<std::array<char, MAX_BYTES>, size_t>(
        {'=', '=', '=', '='}, 0);
    auto &aOut = aRes.first;  // output
    int aPos{0};              // position in output
    unsigned char aSextet{0}; // current working char
    int aSUse{0};             // bits already used in working sextet
    for (int i = 0; i < theCodePoint.size; ++i) {
      unsigned char aData = static_cast<unsigned char>(theCodePoint[i]);
      unsigned char aLeft = (aData >> (2 + aSUse));
      aSextet |= aLeft;
      aOut[aPos++] = base64Enc(aSextet);
      aSextet = 0;
      aSUse += 2;
      unsigned char aRight = aData & ((1 << aSUse) - 1);
      aSextet |= aRight;
      if (aSUse == 6) {
        aOut[aPos++] = base64Enc(aSextet);
        aSUse = 0;
        aSextet = 0;
      } else {
        aSextet = (aSextet << (6 - aSUse)) & 0x3f;
      }
    }
    if (aSUse != 0)
      aOut[aPos++] = base64Enc(aSextet);
    aRes.second = theCodePoint.size;
    return aRes;
  }
};
} // namespace cjson
#endif // CONSTEXPR_JSON_EXT_BASE64_H
