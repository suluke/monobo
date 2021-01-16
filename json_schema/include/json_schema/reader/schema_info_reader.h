#ifndef JSON_SCHEMA_READER_SCHEMA_INFO_READER_H
#define JSON_SCHEMA_READER_SCHEMA_INFO_READER_H

#include "json_schema/reader/applicator.h"
#include "json_schema/reader/compat.h"
#include "json_schema/reader/content.h"
#include "json_schema/reader/core.h"
#include "json_schema/reader/format.h"
#include "json_schema/reader/metadata.h"
#include "json_schema/reader/validation.h"
#include "json_schema/schema_info.h"

namespace json_schema {
template <typename JSON_, typename ErrorHandling_, typename... Sections>
class SchemaInfoReaderBase {
public:
  using Self = SchemaInfoReaderBase;
  using JSON = JSON_;
  using ErrorHandling = ErrorHandling_;
  using InfoMaybe = std::optional<SchemaInfo>;
  using ErrorOrInfoMaybe = typename ErrorHandling::template ErrorOr<InfoMaybe>;

  static constexpr typename ErrorHandling::template ErrorOr<SchemaInfo>
  read(const JSON &theJson) {
    SchemaInfo aResult;
    aResult.NUM_SCHEMAS = 1;
    using TypeEnum = decltype(std::declval<JSON>().getType());
    switch (theJson.getType()) {
    case TypeEnum::BOOL:
      // Nothing to do at all
      break;
    case TypeEnum::OBJECT: {
      const auto aObj = theJson.toObject();
      for (const auto &aKVPair : aObj) {
        const auto aReadResOrError =
            readSections<Sections...>(aKVPair.first, aKVPair.second);
        if (ErrorHandling::isError(aReadResOrError))
          return ErrorHandling::template convertError<SchemaInfo>(
              aReadResOrError);
        const auto aReadRes = ErrorHandling::unwrap(aReadResOrError);
        if (!aReadRes)
          return makeError("Unknown schema entity encountered");
        aResult += *aReadRes;
      }
      break;
    }
    default:
      return makeError(
          "Schema MUST be of either type bool or object (2019-09/Core:4.3.)");
    }

    return aResult;
  }

private:
  static constexpr typename ErrorHandling::template ErrorOr<SchemaInfo>
  makeError(const char *const) {
    return ErrorHandling::template makeError<SchemaInfo>();
  }

  template <typename Section, typename... MoreSections>
  static constexpr std::enable_if_t<sizeof...(MoreSections) != 0,
                                    ErrorOrInfoMaybe>
  readSections(const std::string_view &theKey, const JSON &theJson) {
    const auto aResultOrError =
        Section::template readItem<Self>(theKey, theJson);
    if (ErrorHandling::isError(aResultOrError))
      return aResultOrError;
    const auto aResult = ErrorHandling::unwrap(aResultOrError);
    if (aResult)
      return *aResult;
    return readSections<MoreSections...>(theKey, theJson);
  }
  template <typename Section>
  static constexpr ErrorOrInfoMaybe readSections(const std::string_view &theKey,
                                                 const JSON &theJson) {
    const auto aResultOrError =
        Section::template readItem<Self>(theKey, theJson);
    if (ErrorHandling::isError(aResultOrError))
      return aResultOrError;
    const auto aResult = ErrorHandling::unwrap(aResultOrError);
    if (aResult)
      return *aResult;
    return makeError("Unknown schema entity encountered");
  }
};

template <typename JSON, typename ErrorHandling>
using SchemaInfoReader =
    SchemaInfoReaderBase<JSON, ErrorHandling, ReaderApplicator::InfoReader,
                         ReaderCore::InfoReader, ReaderFormat::InfoReader,
                         ReaderMetadata::InfoReader,
                         ReaderValidation::InfoReader,
                         ReaderContent::InfoReader, ReaderCompat::InfoReader>;

} // namespace json_schema
#endif // JSON_SCHEMA_READER_SCHEMA_INFO_READER_H
