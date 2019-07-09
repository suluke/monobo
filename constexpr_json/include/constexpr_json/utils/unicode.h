#ifndef CONSTEXPR_JSON_UTILS_UNICODE_H
#define CONSTEXPR_JSON_UTILS_UNICODE_H

#include "constexpr_json/utils/parsing.h"

namespace cjson {
struct Utf8 {
  using CodePointTy = uint64_t;
  template <size_t N>
  static constexpr std::pair<CodePointTy, size_t>
  decodeFirst(StrLiteralRef<N> theString) {
    static_assert(N > 0, "Cannot decode empty string");
    unsigned char aFirstByte =
        *reinterpret_cast<const unsigned char *>(&theString[0]);
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
    if (aNumExtraBytes > N - 1)
      // Error: Not enough bytes in string
      return std::make_pair(CodePointTy{0}, size_t{0});
    unsigned aNumBitsFromFirstByte = 6u - aNumExtraBytes;
    unsigned aFirstByteMask = 0xffu >> (8u - aNumBitsFromFirstByte);
    aCP &= aFirstByteMask;
    for (unsigned i = 0; i < aNumExtraBytes; ++i) {
      unsigned char aByte =
          *reinterpret_cast<const unsigned char *>(&theString[1 + i]);
      if ((aByte & 0xc0u) != 0x80u)
        // Error: Not a continuation byte
        return std::make_pair(CodePointTy{0}, size_t{0});
      aCP <<= 6u;
      aCP |= (aByte & 0x3fu);
    }
    return std::make_pair(aCP, size_t{1 + aNumExtraBytes});
  }
};
} // namespace cjson
#endif // CONSTEXPR_JSON_UTILS_UNICODE_H
