#ifndef JSON_SCHEMA_MODEL_CORE_CORE_H
#define JSON_SCHEMA_MODEL_CORE_CORE_H

#include "json_schema/model/core/defs.h"
#include "json_schema/model/core/vocabulary.h"
#include "json_schema/model/format/uri.h"
#include "json_schema/model/format/uri_reference.h"

namespace json_schema {
template <typename Storage> class SchemaCore {
public:
  constexpr SchemaCore() = default;
  constexpr SchemaCore(const SchemaCore &) = default;
  constexpr SchemaCore(SchemaCore &&) = default;
  constexpr SchemaCore &operator=(const SchemaCore &) = default;
  constexpr SchemaCore &operator=(SchemaCore &&) = default;

  using Uri = json_schema::Uri<Storage>;
  using UriReference = json_schema::UriReference<Storage>;

  UriReference itsId{};
  Uri itsSchema{};
  typename Storage::String itsAnchor{};
  // FIXME variant::operator= issue
  // std::variant<UriReference, typename Storage::Schema> itsRef{};
  UriReference itsRef{};
  // std::optional<typename Storage::Schema> itsRefResolved{};
  UriReference itsRecursiveRef{};
  bool itsRecursiveAnchor{};
  Vocabulary<Storage> itsVocabulary{};
  typename Storage::String itsComment{};
  Defs<Storage> itsDefs{};
};
} // namespace json_schema
#endif // JSON_SCHEMA_MODEL_CORE_CORE_H
