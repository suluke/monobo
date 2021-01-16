#ifndef JSON_SCHEMA_MODEL_FORMAT_URI_H
#define JSON_SCHEMA_MODEL_FORMAT_URI_H

#include <string>

namespace json_schema {
template <typename Storage> class Uri {
public:
  constexpr Uri() = default;
  constexpr Uri(const Uri &) = default;
  constexpr Uri(Uri &&) = default;
  constexpr Uri &operator=(const Uri &) = default;
  constexpr Uri &operator=(Uri &&) = default;

  constexpr Uri &operator=(const typename Storage::String &theUri) {
    itsUri = theUri;
    return *this;
  }

  constexpr typename Storage::String toString() const {
    return itsUri;
  }

private:
  typename Storage::String itsUri{};
};
} // namespace json_schema
#endif // JSON_SCHEMA_MODEL_FORMAT_URI_H
