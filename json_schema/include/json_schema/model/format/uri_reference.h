#ifndef JSON_SCHEMA_MODEL_FORMAT_URI_REFERENCE_H
#define JSON_SCHEMA_MODEL_FORMAT_URI_REFERENCE_H

#include <string>

namespace json_schema {
template<typename Storage>
class UriReference {
public:
  constexpr UriReference() = default;
  constexpr UriReference(const UriReference &) = default;
  constexpr UriReference(UriReference &&) = default;
  constexpr UriReference& operator=(const UriReference &) = default;
  constexpr UriReference& operator=(UriReference &&) = default;

  constexpr UriReference& operator=(const typename Storage::String &theUriRef) {
    itsUriRef = theUriRef;
    return *this;
  }

  constexpr typename Storage::String toString() const {
    return itsUriRef;
  }

private:
  typename Storage::String itsUriRef{};
};
} // namespace json_schema
#endif // JSON_SCHEMA_MODEL_FORMAT_URI_REFERENCE_H
