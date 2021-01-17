#ifndef JSON_SCHEMA_MODEL_APPLICATOR_APPLICATOR_H
#define JSON_SCHEMA_MODEL_APPLICATOR_APPLICATOR_H

namespace json_schema {
template <typename Storage> class SchemaApplicator {
public:
  using SchemaRef = typename Storage::Schema;
  using SchemaBuffer = typename Storage::template Buffer<SchemaRef>;
  using SchemaMap =
      typename Storage::template Map<typename Storage::String, SchemaRef>;

  constexpr SchemaApplicator() = default;
  constexpr SchemaApplicator(const SchemaApplicator &) = default;
  constexpr SchemaApplicator(SchemaApplicator &&) = default;
  constexpr SchemaApplicator& operator=(const SchemaApplicator &) = default;
  constexpr SchemaApplicator& operator=(SchemaApplicator &&) = default;

// private:
  SchemaRef itsAdditionalItems{};
  SchemaRef itsUnevaluatedItems{};
  SchemaBuffer itsItems{};
  SchemaRef itsContains{};
  SchemaRef itsAdditionalProperties{};
  SchemaRef itsUnevaluatedProperties{};
  SchemaMap itsProperties{};
  SchemaMap itsPatternProperties{};
  SchemaMap itsDependentSchemas{};
  SchemaRef itsPropertyNames{};
  SchemaRef itsIf{};
  SchemaRef itsThen{};
  SchemaRef itsElse{};
  SchemaBuffer itsAllOf{};
  SchemaBuffer itsAnyOf{};
  SchemaBuffer itsOneOf{};
  SchemaRef itsNot{};
};
} // namespace json_schema
#endif // JSON_SCHEMA_MODEL_APPLICATOR_APPLICATOR_H
