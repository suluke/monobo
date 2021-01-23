#ifndef JSON_SCHEMA_2019_09_VOCABULARY_H
#define JSON_SCHEMA_2019_09_VOCABULARY_H

namespace json_schema {
template <typename Storage> class Vocabulary {
public:
  using Entry =
      typename Storage::template MapEntry<typename Storage::String, bool>;
  using EntryMap =
      typename Storage::template Map<typename Storage::String, bool>;
  constexpr Vocabulary() = default;
  constexpr Vocabulary(const Vocabulary &) = default;
  constexpr Vocabulary(Vocabulary &&) = default;
  constexpr Vocabulary &operator=(const Vocabulary &) = default;
  constexpr Vocabulary &operator=(Vocabulary &&) = default;
  constexpr Vocabulary(EntryMap &&theEntries) : itsEntries{std::move(theEntries)} {}
  constexpr Vocabulary &operator=(EntryMap &&theEntries) {
    itsEntries = std::move(theEntries);
    return *this;
  }

private:
  EntryMap itsEntries;
};
} // namespace json_schema
#endif // JSON_SCHEMA_2019_09_VOCABULARY_H
