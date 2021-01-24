#ifndef JSON_SCHEMA_2019_09_MODEL_DEFS_H
#define JSON_SCHEMA_2019_09_MODEL_DEFS_H

namespace json_schema {
template <typename Storage> class Defs {
public:
  using Entry = typename Storage::template MapEntry<typename Storage::String,
                                                    typename Storage::Schema>;
  using EntryMap = typename Storage::template Map<typename Storage::String,
                                                  typename Storage::Schema>;
  constexpr Defs() = default;
  constexpr Defs(const Defs &) = default;
  constexpr Defs(EntryMap &&theEntries) : itsEntries(std::move(theEntries)) {}
  constexpr Defs(Defs &&) = default;
  constexpr Defs &operator=(const Defs &) = default;
  constexpr Defs &operator=(Defs &&) = default;
  constexpr Defs &operator=(EntryMap &&theEntries) {
    itsEntries = std::move(theEntries);
    return *this;
  }
  constexpr EntryMap toDict() const { return itsEntries; }

  template <typename Context> class Accessor {
  public:
    constexpr Accessor(const Context &theContext, const Defs &theDefs)
        : itsContext(&theContext), itsDefs(theDefs) {}

    using MapAccessor =
        typename Context::template MapAccessor<typename Storage::String,
                                               typename Storage::Schema>;

    constexpr MapAccessor toDict() const {
      return MapAccessor{*itsContext, itsDefs.toDict()};
    }

  private:
    const Context *itsContext{};
    Defs itsDefs;
  };

private:
  EntryMap itsEntries;
};
} // namespace json_schema
#endif // JSON_SCHEMA_2019_09_MODEL_DEFS_H
