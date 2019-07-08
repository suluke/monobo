#ifndef CONSTEXPR_JSON_UTILS_MAP_H
#define CONSTEXPR_JSON_UTILS_MAP_H

#include <optional>

namespace cjson {
template <typename KeyTy, typename ValTy, size_t NumEntries = 0> struct map {
  constexpr std::optional<map<KeyTy, ValTy, NumEntries + 1>>
  insert(const KeyTy &theKey, const ValTy &theVal) {
    return std::nullopt;
  }
  constexpr std::optional<map<KeyTy, ValTy, NumEntries - 1>>
  erase(const KeyTy &theKey) {
    return std::nullopt;
  }
  constexpr bool contains(const KeyTy &theKey) { return false; }
};
} // namespace cjson
#endif // CONSTEXPR_JSON_UTILS_MAP_H