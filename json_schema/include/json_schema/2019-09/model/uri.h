#ifndef JSON_SCHEMA_2019_09_URI_H
#define JSON_SCHEMA_2019_09_URI_H

namespace json_schema {
template <typename Storage> class Uri {
public:
  constexpr Uri() = default;
  constexpr Uri(const Uri &) = default;
  constexpr Uri(Uri &&) = default;
  constexpr Uri &operator=(const Uri &) = default;
  constexpr Uri &operator=(Uri &&) = default;

  constexpr Uri &operator=(const typename Storage::StringPtr &theUri) {
    itsUri = theUri;
    return *this;
  }

  constexpr typename Storage::StringPtr toString() const { return itsUri; }

  template <typename Context> class Accessor {
  public:
    constexpr Accessor(const Context &theContext, const Uri &theUri)
        : itsContext(&theContext), itsUri(theUri) {}

    constexpr std::optional<std::string_view> toString() const {
      if (!itsUri.toString())
        return std::nullopt;
      return itsContext->getString(*itsUri.toString());
    }

  private:
    const Context *itsContext{};
    Uri itsUri;
  };

private:
  typename Storage::StringPtr itsUri{};
};
} // namespace json_schema
#endif // JSON_SCHEMA_2019_09_URI_H
