#ifndef JSON_SCHEMA_2019_09_MODEL_CORE_H
#define JSON_SCHEMA_2019_09_MODEL_CORE_H

#include "json_schema/2019-09/model/defs.h"
#include "json_schema/2019-09/model/uri.h"
#include "json_schema/2019-09/model/uri_reference.h"
#include "json_schema/2019-09/model/vocabulary.h"

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
    using UriAccessor = typename Uri::template Accessor<Context>;
    using VocabularyAccessor =
        typename Vocabulary<Storage>::template Accessor<Context>;
    using DefsAccessor = typename Defs<Storage>::template Accessor<Context>;

    constexpr Accessor(const Context &theContext, const SchemaCore &theCore)
        : itsContext(&theContext), itsCore(&theCore) {}
    constexpr const UriRefAccessor getId() const {
      return UriRefAccessor{*itsContext, itsCore->itsId};
    }
    constexpr const UriAccessor getSchema() const {
      return UriAccessor{*itsContext, itsCore->itsSchema};
    }
    constexpr const VocabularyAccessor getVocabulary() const {
      return VocabularyAccessor{*itsContext, itsCore->itsVocabulary};
    }
    constexpr std::string_view getAnchor() const {
      return itsContext->getString(itsCore->itsAnchor);
    }
    constexpr const UriRefAccessor getRef() const {
      return UriRefAccessor{*itsContext, itsCore->itsRef};
    }
    constexpr const UriRefAccessor getRecursiveRef() const {
      return UriRefAccessor{*itsContext, itsCore->itsRecursiveRef};
    }
    constexpr bool getRecursiveAnchor() const {
      return itsCore->itsRecursiveAnchor;
    }
    constexpr std::string_view getComment() const {
      return itsContext->getString(itsCore->itsComment);
    }
    constexpr const DefsAccessor getDefs() const {
      return DefsAccessor{*itsContext, itsCore->itsDefs};
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
#endif // JSON_SCHEMA_2019_09_MODEL_CORE_H
