#ifndef JSON_SCHEMA_2019_09_APPLICATOR_H
#define JSON_SCHEMA_2019_09_APPLICATOR_H

namespace json_schema {
template <typename Storage> class SchemaApplicator {
public:
  using SchemaRef = typename Storage::Schema;
  using SchemaList = typename Storage::template Buffer<SchemaRef>;
  using SchemaDict =
      typename Storage::template Map<typename Storage::String, SchemaRef>;

  constexpr SchemaApplicator() = default;
  constexpr SchemaApplicator(const SchemaApplicator &) = default;
  constexpr SchemaApplicator(SchemaApplicator &&) = default;
  constexpr SchemaApplicator &operator=(const SchemaApplicator &) = default;
  constexpr SchemaApplicator &operator=(SchemaApplicator &&) = default;

  // private:
  SchemaRef itsAdditionalItems{};
  SchemaRef itsUnevaluatedItems{};
  SchemaList itsItems{};
  SchemaRef itsContains{};
  SchemaRef itsAdditionalProperties{};
  SchemaRef itsUnevaluatedProperties{};
  SchemaDict itsProperties{};
  SchemaDict itsPatternProperties{};
  SchemaDict itsDependentSchemas{};
  SchemaRef itsPropertyNames{};
  SchemaRef itsIf{};
  SchemaRef itsThen{};
  SchemaRef itsElse{};
  SchemaList itsAllOf{};
  SchemaList itsAnyOf{};
  SchemaList itsOneOf{};
  SchemaRef itsNot{};
};
} // namespace json_schema
#endif // JSON_SCHEMA_2019_09_APPLICATOR_H
