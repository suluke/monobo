#ifndef JSON_SCHEMA_2019_09_MODEL_CONTENT_H
#define JSON_SCHEMA_2019_09_MODEL_CONTENT_H

namespace json_schema {
template <typename Storage> class SchemaContent {
public:
  using SchemaRef = typename Storage::Schema;
  using StringRef = typename Storage::String;

  constexpr SchemaContent() = default;
  constexpr SchemaContent(const SchemaContent &) = default;
  constexpr SchemaContent(SchemaContent &&) = default;
  constexpr SchemaContent &operator=(const SchemaContent &) = default;
  constexpr SchemaContent &operator=(SchemaContent &&) = default;

  template <typename Context> class Accessor {
  public:
    using SchemaAccessor = std::variant<bool, SchemaObjectAccessor<Context>>;

    constexpr Accessor(const Context &theContext,
                       const SchemaContent &theContent)
        : itsContext(&theContext), itsContent(&theContent) {}

    constexpr std::string_view getContentMediaType() const {
      return itsContext->getString(itsContent->itsContentMediaType);
    }
    constexpr std::string_view getContentEncoding() const {
      return itsContext->getString(itsContent->itsContentEncoding);
    }

    constexpr SchemaAccessor getContentSchema() const {
      if (itsContent->itsContentSchema == itsContext->getTrueSchemaRef())
        return true;
      return SchemaObjectAccessor<Context>{*itsContext,
                                           itsContent->itsContentSchema};
    }

  private:
    const Context *itsContext;
    const SchemaContent *itsContent;
  };

  // private:
  StringRef itsContentMediaType;
  StringRef itsContentEncoding;
  SchemaRef itsContentSchema;
};
} // namespace json_schema
#endif // JSON_SCHEMA_2019_09_MODEL_CONTENT_H
