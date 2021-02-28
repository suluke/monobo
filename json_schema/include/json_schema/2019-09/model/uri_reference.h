#ifndef JSON_SCHEMA_2019_09_MODEL_URI_REFERENCE_H
#define JSON_SCHEMA_2019_09_MODEL_URI_REFERENCE_H

#include <string_view>

namespace json_schema {
template <typename Storage> class UriReference {
public:
  constexpr UriReference() = default;
  constexpr UriReference(const UriReference &) = default;
  constexpr UriReference(UriReference &&) = default;
  constexpr UriReference &operator=(const UriReference &) = default;
  constexpr UriReference &operator=(UriReference &&) = default;

  constexpr UriReference &
  operator=(const typename Storage::StringPtr &theUriRef) {
    itsUriRef = theUriRef;
    return *this;
  }

  constexpr typename Storage::StringPtr toString() const { return itsUriRef; }

  template <typename Context> class Accessor {
  public:
    constexpr Accessor(const Context &theContext, const UriReference &theUriRef)
        : itsContext(&theContext), itsUriRef(theUriRef) {}

    constexpr std::optional<std::string_view> toString() const {
      if (!itsUriRef.toString())
        return std::nullopt;
      return itsContext->getString(*itsUriRef.toString());
    }

  private:
    const Context *itsContext{};
    UriReference itsUriRef;
  };

private:
  typename Storage::StringPtr itsUriRef{};
};
} // namespace json_schema
#endif // JSON_SCHEMA_2019_09_MODEL_URI_REFERENCE_H
