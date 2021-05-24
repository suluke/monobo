#ifndef JSON_SCHEMA_2019_09_MODEL_METADATA_H
#define JSON_SCHEMA_2019_09_MODEL_METADATA_H

#include "json_schema/modelling.h"

namespace json_schema {
template <typename Storage> class SchemaMetadata {
public:
  using StringPtr = typename Storage::StringPtr;
  using JsonPtr = typename Storage::JsonPtr;
  using JsonListPtr =
      typename Storage::template BufferPtr<typename Storage::JsonRef>;

  constexpr SchemaMetadata() = default;
  constexpr SchemaMetadata(const SchemaMetadata &) = default;
  constexpr SchemaMetadata(SchemaMetadata &&) = default;
  constexpr SchemaMetadata &operator=(const SchemaMetadata &) = default;
  constexpr SchemaMetadata &operator=(SchemaMetadata &&) = default;

  template <typename Context> class Accessor {
  public:
    using JsonListAccessor =
        typename Context::template BufferAccessor<typename Storage::JsonRef>;
    using JsonListAccessorMaybe = std::optional<JsonListAccessor>;

    constexpr Accessor(const Context &theContext,
                       const SchemaMetadata &theMetadata)
        : itsContext(&theContext), itsMetadata(&theMetadata) {}

    constexpr std::optional<std::string_view> getTitle() const {
      if (!itsMetadata->itsTitle)
        return std::nullopt;
      return itsContext->getString(*itsMetadata->itsTitle);
    }
    constexpr std::optional<std::string_view> getDescription() const {
      if (!itsMetadata->itsDescription)
        return std::nullopt;
      return itsContext->getString(*itsMetadata->itsDescription);
    }

    constexpr std::optional<typename Context::JsonAccessor> getDefault() const {
      if (!itsMetadata->itsDefault)
        return std::nullopt;
      return itsContext->getJson(*itsMetadata->itsDefault);
    }

    constexpr auto getDeprecated() const { return itsMetadata->itsDeprecated; }
    constexpr auto getReadOnly() const { return itsMetadata->itsReadOnly; }
    constexpr auto getWriteOnly() const { return itsMetadata->itsWriteOnly; }

    constexpr JsonListAccessorMaybe getExamples() const {
      if (!itsMetadata->itsExamples)
        return std::nullopt;
      return JsonListAccessor{*itsContext, *itsMetadata->itsExamples};
    }

  private:
    const Context *itsContext;
    const SchemaMetadata *itsMetadata;
  };

  // private:
  StringPtr itsTitle{};
  StringPtr itsDescription{};
  JsonPtr itsDefault{};
  bool itsDeprecated{false};
  bool itsReadOnly{false};
  bool itsWriteOnly{false};
  JsonListPtr itsExamples{};
};
} // namespace json_schema
#endif // JSON_SCHEMA_2019_09_MODEL_METADATA_H
