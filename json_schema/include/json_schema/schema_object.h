#ifndef JSON_SCHEMA_SCHEMA_OBJECT_H
#define JSON_SCHEMA_SCHEMA_OBJECT_H

namespace json_schema {

template <typename Storage, template<typename> typename... Sections>
class SchemaObjectBase : private Sections<Storage>... {
public:
  using SchemaCore = json_schema::SchemaCore<Storage>;

  constexpr SchemaObjectBase() = default;
  constexpr SchemaObjectBase(const SchemaObjectBase &) = default;
  constexpr SchemaObjectBase(SchemaObjectBase &&) = default;
  constexpr SchemaObjectBase &operator=(const SchemaObjectBase &) = default;
  constexpr SchemaObjectBase &operator=(SchemaObjectBase &&) = default;

  template <template <typename> typename Section>
  constexpr Section<Storage> &getSection() noexcept {
    return static_cast<Section<Storage> &>(*this);
  }
  template <template <typename> typename Section>
  constexpr const Section<Storage> &getSection() const noexcept {
    return static_cast<const Section<Storage> &>(*this);
  }
};

template <typename Context> class SchemaObjectAccessor {
public:
  using SchemaRef = typename Context::SchemaRef;
  using Storage = typename Context::Storage;
  template <template <typename> typename Section>
  using Accessor = typename Section<Storage>::template Accessor<Context>;

  constexpr SchemaObjectAccessor(const Context &theContext,
                                 const SchemaRef theSchemaObject)
      : itsContext(&theContext), itsSchemaObject(theSchemaObject) {}

  template <template <typename> typename Section>
  constexpr const Accessor<Section> getSection() const noexcept {
    return Accessor<Section>{
        *itsContext,
        itsContext->getSchemaObject(itsSchemaObject).template getSection<Section>()};
  }

private:
  const Context *itsContext{};
  SchemaRef itsSchemaObject;
};

} // namespace json_schema
#endif // JSON_SCHEMA_SCHEMA_OBJECT_H
