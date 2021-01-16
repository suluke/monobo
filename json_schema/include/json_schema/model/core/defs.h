#ifndef JSON_SCHEMA_MODEL_CORE_DEFS_H
#define JSON_SCHEMA_MODEL_CORE_DEFS_H

#include <tuple>

namespace json_schema {
template <typename Storage> class Defs {
public:
  using Entry =
      typename Storage::template MapEntry<typename Storage::String, typename Storage::Schema>;
  using EntryMap =
      typename Storage::template Map<typename Storage::String, typename Storage::Schema>;
  constexpr Defs() = default;
  constexpr Defs(const Defs &) = default;
  constexpr Defs(EntryMap && theEntries) : itsEntries(std::move(theEntries)) {}
  constexpr Defs(Defs &&) = default;
  constexpr Defs& operator=(const Defs &) = default;
  constexpr Defs& operator=(Defs &&) = default;
  constexpr Defs& operator=(EntryMap &&theEntries) {
    itsEntries = std::move(theEntries);
    return *this;
  }

private:
  EntryMap itsEntries;
};
} // namespace json_schema
#endif // JSON_SCHEMA_MODEL_CORE_DEFS_H
