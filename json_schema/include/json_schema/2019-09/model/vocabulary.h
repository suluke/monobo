#ifndef JSON_SCHEMA_2019_09_MODEL_VOCABULARY_H
#define JSON_SCHEMA_2019_09_MODEL_VOCABULARY_H

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
  constexpr Vocabulary(EntryMap &&theEntries)
      : itsEntries{std::move(theEntries)} {}
  constexpr Vocabulary &operator=(EntryMap &&theEntries) {
    itsEntries = std::move(theEntries);
    return *this;
  }
  constexpr EntryMap toDict() const { return itsEntries; }

  template <typename Context> class Accessor {
  public:
    constexpr Accessor(const Context &theContext, const Vocabulary &theVocab)
        : itsContext(&theContext), itsVocab(theVocab) {}

    using MapAccessor =
        typename Context::template MapAccessor<typename Storage::String, bool>;

    constexpr MapAccessor toDict() const {
      return MapAccessor{*itsContext, itsVocab.toDict()};
    }

  private:
    const Context *itsContext{};
    Vocabulary itsVocab;
  };

private:
  EntryMap itsEntries;
};
} // namespace json_schema
#endif // JSON_SCHEMA_2019_09_MODEL_VOCABULARY_H
