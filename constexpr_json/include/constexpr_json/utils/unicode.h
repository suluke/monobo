#ifndef CONSTEXPR_JSON_UTILS_UNICODE_H
#define CONSTEXPR_JSON_UTILS_UNICODE_H

#include "constexpr_json/utils/parsing.h"
#include <array>

namespace cjson {
struct Utf8 {
  using CodePointTy = uint64_t;
  static constexpr std::pair<CodePointTy, size_t>
  decodeFirst(std::string_view theString) {
    if (theString.empty())
      // Cannot decode empty string"
      return std::make_pair(CodePointTy{0}, size_t{0});
    unsigned char aFirstByte = static_cast<unsigned char>(theString[0]);
    std::array<bool, 5> aInterestingBits{
        {static_cast<bool>(aFirstByte & 0x80u),
         static_cast<bool>(aFirstByte & 0x40u),
         static_cast<bool>(aFirstByte & 0x20u),
         static_cast<bool>(aFirstByte & 0x10u),
         static_cast<bool>(aFirstByte & 0x08u)}};
    CodePointTy aCP = static_cast<CodePointTy>(aFirstByte);
    if (!aInterestingBits[0])
      // No continuation bytes
      return std::make_pair(aCP, size_t{1});
    if (!aInterestingBits[1])
      // Error: unexpected continuation byte
      return std::make_pair(CodePointTy{0}, size_t{0});
    unsigned aNumExtraBytes =
        (!aInterestingBits[2])
            ? 1
            : ((!aInterestingBits[3]) ? 2 : ((!aInterestingBits[3]) ? 3 : 0));
    if (!aNumExtraBytes)
      // Error: Illegal first byte
      return std::make_pair(CodePointTy{0}, size_t{0});
    if (aNumExtraBytes > theString.size() - 1)
      // Error: Not enough bytes in string
      return std::make_pair(CodePointTy{0}, size_t{0});
    unsigned aNumBitsFromFirstByte = 6u - aNumExtraBytes;
    unsigned aFirstByteMask = 0xffu >> (8u - aNumBitsFromFirstByte);
    aCP &= aFirstByteMask;
    for (unsigned i = 0; i < aNumExtraBytes; ++i) {
      unsigned char aByte = static_cast<unsigned char>(theString[1 + i]);
      if ((aByte & 0xc0u) != 0x80u)
        // Error: Not a continuation byte
        return std::make_pair(CodePointTy{0}, size_t{0});
      aCP <<= 6u;
      aCP |= (aByte & 0x3fu);
    }
    return std::make_pair(aCP, size_t{1 + aNumExtraBytes});
  }

  static constexpr size_t MAX_BYTES = 4;
  static constexpr std::pair<std::array<char, MAX_BYTES>, ssize_t>
  encode(CodePointTy theCodePoint) {
    constexpr std::pair<std::array<char, MAX_BYTES>, ssize_t> aErrorResult =
        std::make_pair(std::array<char, MAX_BYTES>{}, -1);
    if (theCodePoint <= 0x7fu)
      return std::make_pair(std::array<char, MAX_BYTES>(
                                {static_cast<char>(theCodePoint), 0, 0, 0}),
                            1);
    if (theCodePoint <= 0x7ffu)
      return std::make_pair(
          std::array<char, MAX_BYTES>(
              {static_cast<char>(0xc0u | (theCodePoint >> 6)),
               static_cast<char>(0x80u | (theCodePoint & 0x3fu)), 0, 0}),
          2);
    if (theCodePoint <= 0xffffu)
      return std::make_pair(
          std::array<char, MAX_BYTES>(
              {static_cast<char>(0xe0u | (theCodePoint >> 12)),
               static_cast<char>(0x80u | ((theCodePoint >> 6) & 0x3fu)),
               static_cast<char>(0x80u | (theCodePoint & 0x3fu)), 0}),
          3);
    if (theCodePoint <= 0x10ffffu)
      return std::make_pair(
          std::array<char, MAX_BYTES>(
              {static_cast<char>(0xf0u | (theCodePoint >> 18)),
               static_cast<char>(0x80u | ((theCodePoint >> 12) & 0x3fu)),
               static_cast<char>(0x80u | ((theCodePoint >> 6) & 0x3fu)),
               static_cast<char>(0x80u | (theCodePoint & 0x3fu))}),
          4);
    return aErrorResult;
  }
};
} // namespace cjson
#endif // CONSTEXPR_JSON_UTILS_UNICODE_H
