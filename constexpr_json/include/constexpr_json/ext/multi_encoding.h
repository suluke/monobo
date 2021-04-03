#ifndef CONSTEXPR_JSON_EXT_MULTI_ENCODING_H
#define CONSTEXPR_JSON_EXT_MULTI_ENCODING_H

#include <algorithm>
#include <array>
#include <string_view>
#include <variant>

namespace cjson {
/// Generic encoding type which can be switched at runtime between one of the
/// specified template param encodings.
template <typename... Encodings> struct MultiEncoding {
  template <typename Encoding>
  constexpr MultiEncoding(const Encoding &theEncoding)
      : itsEncoding{theEncoding} {}
  constexpr MultiEncoding(const MultiEncoding &) = default;
  constexpr MultiEncoding(MultiEncoding &&) = default;
  constexpr MultiEncoding &operator=(const MultiEncoding &) = default;
  constexpr MultiEncoding &operator=(MultiEncoding &&) = default;

  using CodePointTy = std::common_type_t<typename Encodings::CodePointTy...>;
  static constexpr size_t MAX_BYTES = std::max({Encodings::MAX_BYTES...});

  constexpr std::pair<CodePointTy, size_t>
  decodeFirst(std::string_view theString) const noexcept {
    return std::visit(
        [theString](const auto &aEncoding) {
          const auto aResult = aEncoding.decodeFirst(theString);
          return std::make_pair(static_cast<CodePointTy>(aResult.first),
                                aResult.second);
        },
        itsEncoding);
  }

  constexpr std::pair<std::array<char, MAX_BYTES>, size_t>
  encode(CodePointTy theCodePoint) const noexcept {
    return std::visit(
        [theCodePoint](const auto &aEncoding) {
          const auto aResult = aEncoding.encode(theCodePoint);
          std::array<char, MAX_BYTES> aEncBuf{};
          for (int aIdx = 0; aIdx < aResult.first.size(); ++aIdx)
            aEncBuf[aIdx] = aResult.first[aIdx];
          return std::make_pair(aEncBuf, aResult.second);
        },
        itsEncoding);
  }

private:
  std::variant<Encodings...> itsEncoding;
};
} // namespace cjson
#endif // CONSTEXPR_JSON_EXT_MULTI_ENCODING_H
