#ifndef CONSTEXPR_JSON_UTILS_PARSING_H
#define CONSTEXPR_JSON_UTILS_PARSING_H

namespace cjson {
template <size_t N> using StrLiteralRef = const char (&)[N];

template <size_t N, typename EncodingTy>
constexpr std::pair<double, size_t> parseDouble(StrLiteralRef<N> theString) {
  return std::make_pair(0., -1);
}
} // namespace cjson
#endif // CONSTEXPR_JSON_UTILS_PARSING_H