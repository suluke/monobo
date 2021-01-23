#ifndef JSON_SCHEMA_2019_09_CORE_H
#define JSON_SCHEMA_2019_09_CORE_H

#include "json_schema/2019-09/defs.h"
#include "json_schema/2019-09/vocabulary.h"
#include "json_schema/2019-09/uri.h"
#include "json_schema/2019-09/uri_reference.h"

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

  template <typename Context> class Accessor {
  public:
    using UriRefAccessor = typename UriReference::template Accessor<Context>;

    constexpr Accessor(const Context &theContext, const SchemaCore &theCore)
        : itsContext(&theContext), itsCore(&theCore) {}
    constexpr const UriRefAccessor getId() const {
      return UriRefAccessor{*itsContext, itsCore->itsId};
    }

  private:
    const Context *itsContext;
    const SchemaCore *itsCore;
  };

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
#endif // JSON_SCHEMA_2019_09_CORE_H
