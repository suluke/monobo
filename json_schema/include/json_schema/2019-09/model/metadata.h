#ifndef JSON_SCHEMA_2019_09_MODEL_METADATA_H
#define JSON_SCHEMA_2019_09_MODEL_METADATA_H

namespace json_schema {
template <typename Storage> class SchemaMetadata {
public:
  using StringRef = typename Storage::String;
  using JsonRef = typename Storage::Json;
  using JsonListRef = typename Storage::template Buffer<JsonRef>;

  constexpr SchemaMetadata() = default;
  constexpr SchemaMetadata(const SchemaMetadata &) = default;
  constexpr SchemaMetadata(SchemaMetadata &&) = default;
  constexpr SchemaMetadata &operator=(const SchemaMetadata &) = default;
  constexpr SchemaMetadata &operator=(SchemaMetadata &&) = default;

  template <typename Context> class Accessor {
  public:
    using JsonListAccessor =
        typename Context::template BufferAccessor<typename Storage::Json>;

    constexpr Accessor(const Context &theContext,
                       const SchemaMetadata &theMetadata)
        : itsContext(&theContext), itsMetadata(&theMetadata) {}

    constexpr std::string_view getTitle() const {
      return itsContext->getString(itsMetadata->itsTitle);
    }
    constexpr std::string_view getDescription() const {
      return itsContext->getString(itsMetadata->itsDescription);
    }

    constexpr std::optional<typename Context::JsonAccessor> getDefault() const {
      if (itsMetadata->itsDefault.isValid())
        return itsContext->getJson(itsMetadata->itsDefault);
      return std::nullopt;
    }

    constexpr bool getDeprecated() const { return itsMetadata->itsDeprecated; }
    constexpr bool getReadOnly() const { return itsMetadata->itsReadOnly; }
    constexpr bool getWriteOnly() const { return itsMetadata->itsWriteOnly; }

    constexpr JsonListAccessor getExamples() const {
      return JsonListAccessor{*itsContext, itsMetadata->itsExamples};
    }

  private:
    const Context *itsContext;
    const SchemaMetadata *itsMetadata;
  };

  // private:
  StringRef itsTitle{};
  StringRef itsDescription{};
  JsonRef itsDefault{};
  bool itsDeprecated = false;
  bool itsReadOnly = false;
  bool itsWriteOnly = false;
  JsonListRef itsExamples{};
};
} // namespace json_schema
#endif // JSON_SCHEMA_2019_09_MODEL_METADATA_H
