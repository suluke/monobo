#ifndef CONSTEXPR_JSON_EXT_ASCII_H
#define CONSTEXPR_JSON_EXT_ASCII_H

#include <array>
#include <string_view>

namespace cjson {
struct Ascii {
  using CodePointTy = char;
  static constexpr size_t MAX_BYTES = 1;

  constexpr std::pair<CodePointTy, size_t>
  decodeFirst(std::string_view theString) const noexcept {
    if (theString.empty())
      return std::make_pair(0, 0);
    return std::make_pair(theString.front(), 1);
  }

  constexpr std::pair<std::array<char, MAX_BYTES>, size_t>
  encode(CodePointTy theCodePoint) const noexcept {
    if (theCodePoint < 0 /*|| static_cast<int>(theCodePoint) > 127*/)
      return std::make_pair(std::array<char, MAX_BYTES>{}, 0);
    return std::make_pair(std::array<char, MAX_BYTES>{theCodePoint}, 1);
  }
};
} // namespace cjson
#endif // CONSTEXPR_JSON_EXT_ASCII_H
