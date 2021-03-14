#ifndef JSON_SCHEMA_2019_09_MODEL_CONTENT_H
#define JSON_SCHEMA_2019_09_MODEL_CONTENT_H

namespace json_schema {
template <typename Storage> class SchemaContent {
public:
  using SchemaPtr = typename Storage::SchemaPtr;
  using StringPtr = typename Storage::StringPtr;

  constexpr SchemaContent() = default;
  constexpr SchemaContent(const SchemaContent &) = default;
  constexpr SchemaContent(SchemaContent &&) = default;
  constexpr SchemaContent &operator=(const SchemaContent &) = default;
  constexpr SchemaContent &operator=(SchemaContent &&) = default;

  template <typename Context> class Accessor {
  public:
    using SchemaAccessor = std::variant<bool, SchemaObjectAccessor<Context>>;
    using SchemaAccessorMaybe = std::optional<SchemaAccessor>;

    constexpr Accessor(const Context &theContext,
                       const SchemaContent &theContent)
        : itsContext(&theContext), itsContent(&theContent) {}

    constexpr std::optional<std::string_view> getContentMediaType() const {
      if (!itsContent->itsContentMediaType)
        return std::nullopt;
      return itsContext->getString(*itsContent->itsContentMediaType);
    }
    constexpr std::optional<std::string_view> getContentEncoding() const {
      if (!itsContent->itsContentEncoding)
        return std::nullopt;
      return itsContext->getString(*itsContent->itsContentEncoding);
    }

    constexpr SchemaAccessorMaybe getContentSchema() const {
      if (!itsContent->itsContentSchema)
        return std::nullopt;
      if (*itsContent->itsContentSchema == itsContext->getTrueSchemaRef())
        return SchemaAccessor{true};
      return SchemaAccessor{SchemaObjectAccessor<Context>{
          *itsContext, *itsContent->itsContentSchema}};
    }

  private:
    const Context *itsContext;
    const SchemaContent *itsContent;
  };

  // private:
  StringPtr itsContentMediaType{};
  StringPtr itsContentEncoding{};
  SchemaPtr itsContentSchema{};
};
} // namespace json_schema
#endif // JSON_SCHEMA_2019_09_MODEL_CONTENT_H
