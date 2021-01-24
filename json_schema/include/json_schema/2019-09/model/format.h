#ifndef JSON_SCHEMA_2019_09_MODEL_FORMAT_H
#define JSON_SCHEMA_2019_09_MODEL_FORMAT_H

namespace json_schema {
template <typename Storage> class SchemaFormat {
public:
  constexpr SchemaFormat() = default;
  constexpr SchemaFormat(const SchemaFormat &) = default;
  constexpr SchemaFormat(SchemaFormat &&) = default;
  constexpr SchemaFormat &operator=(const SchemaFormat &) = default;
  constexpr SchemaFormat &operator=(SchemaFormat &&) = default;

  template <typename Context> class Accessor {
  public:
    constexpr Accessor(const Context &theContext, const SchemaFormat &theFormat)
        : itsContext(&theContext), itsFormat(&theFormat) {}

    constexpr std::string_view getFormat() const {
      return itsContext->getString(itsFormat->itsFormat);
    }

  private:
    const Context *itsContext;
    const SchemaFormat *itsFormat;
  };

  typename Storage::String itsFormat{};
};
} // namespace json_schema
#endif // JSON_SCHEMA_2019_09_MODEL_FORMAT_H
