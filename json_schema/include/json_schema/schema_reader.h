#ifndef JSON_SCHEMA_SCHEMA_READER_H
#define JSON_SCHEMA_SCHEMA_READER_H

#include "constexpr_json/impl/document_allocator.h"
#include "json_schema/model/schema_object.h"
#include "json_schema/reader/applicator.h"
#include "json_schema/reader/compat.h"
#include "json_schema/reader/content.h"
#include "json_schema/reader/core.h"
#include "json_schema/reader/format.h"
#include "json_schema/reader/metadata.h"
#include "json_schema/reader/validation.h"
#include "json_schema/schema_info.h"
#include "json_schema/util.h"

namespace json_schema {
template <typename SchemaContext, typename ErrorHandling_, typename... Sections>
class SchemaReaderBase {
public:
  using Self = SchemaReaderBase;

  using ErrorHandling = ErrorHandling_;
  using Storage = typename SchemaContext::Storage;
  using SchemaRef = typename Storage::Schema;
  using SchemaObject = typename SchemaContext::SchemaObject;
  using ErrorOrConsumed = typename ErrorHandling::template ErrorOr<bool>;

  template <typename... JSONs> struct ReadResult {
    SchemaContext itsContext;
    std::array<SchemaRef, sizeof...(JSONs)> itsSchemas;
  };
  template <typename... JSONs>
  using ReadResultOrError =
      typename ErrorHandling::template ErrorOr<ReadResult<JSONs...>>;

  template <typename JSON, typename... MoreJSONs>
  static constexpr ReadResultOrError<JSON, MoreJSONs...>
  read(const JSON &theJson, MoreJSONs &&...theMoreJsons) {
    SchemaReaderBase aInstance;
    const auto aErrorMaybe =
        aInstance.readMany(theJson, std::forward<MoreJSONs>(theMoreJsons)...);
    if (ErrorHandling::isError(aErrorMaybe))
      return ErrorHandling::template convertError<
          ReadResult<JSON, MoreJSONs...>>(aErrorMaybe);
    return ReadResult<JSON, MoreJSONs...>{std::move(aInstance.itsContext),
                                          ErrorHandling::unwrap(aErrorMaybe)};
  }

  using SchemaAllocator = typename SchemaContext::Allocator;
  using JsonAllocator =
      cjson::DocumentAllocator<typename SchemaContext::JsonStorage,
                               ErrorHandling>;

  constexpr typename Storage::String
  allocateString(const std::string_view &theStr) {
    return itsSchemaAlloc.allocateString(itsContext, theStr);
  }

  template <typename KeyT, typename ValT>
  using Map = typename Storage::template Map<KeyT, ValT>;

  template <typename KeyT, typename ValT>
  constexpr Map<KeyT, ValT> allocateMap(const size_t theSize) {
    return itsSchemaAlloc.allocateMap(itsContext, theSize,
                                      type_tag<KeyT, ValT>{});
  }

  template <typename KeyT, typename ValT>
  constexpr void setMapEntry(Map<KeyT, ValT> &theMap, const ptrdiff_t theIdx,
                             const KeyT &theKey, const ValT &theVal) {
    itsContext.setMapEntry(theMap, theIdx, theKey, theVal);
  }

  template <typename JSON>
  constexpr typename ErrorHandling::template ErrorOr<SchemaRef>
  readSchema(const JSON &theJson) {
    using TypeEnum = decltype(std::declval<JSON>().getType());
    const TypeEnum aType = theJson.getType();
    if (aType == TypeEnum::BOOL) {
      const auto aBoolSchemaVal = theJson.toBool();
      if (!aBoolSchemaVal)
        return ErrorHandling::template makeError<SchemaRef>(
            "Schema value must be `true` in case it is of type bool");
      return itsContext.getTrueSchemaRef();
    } else if (aType == TypeEnum::OBJECT) {
      SchemaObject aSchema;
      for (const auto &[aKey, aValue] : theJson.toObject()) {
        const auto aErrorOrConsumed =
            readSections<JSON, Sections...>(aSchema, aKey, aValue);
        if (ErrorHandling::isError(aErrorOrConsumed))
          return ErrorHandling::template convertError<SchemaRef>(
              aErrorOrConsumed);
        // if (!ErrorHandling::unwrap(aErrorOrConsumed))
        //   return ErrorHandling::template makeError<SchemaRef>(
        //       "Encountered unknown entity in schema");
      }
      return itsSchemaAlloc.allocateSchema(aSchema, itsContext);
    }
    return ErrorHandling::template makeError<SchemaRef>(
        "Schema is neither of type bool nor object");
  }

private:
  SchemaReaderBase() = default;

  template <typename... JSONs>
  using ErrorOrMany = typename ErrorHandling::template ErrorOr<
      std::array<SchemaRef, sizeof...(JSONs)>>;

  template <typename JSON, typename... MoreJSONs>
  constexpr ErrorOrMany<JSON, MoreJSONs...>
  readMany(const JSON &theJson, MoreJSONs &&...theMoreJsons) {
    using OutputTy = std::array<SchemaRef, sizeof...(MoreJSONs) + 1>;
    const auto aResult = readSchema(theJson);
    if (ErrorHandling::isError(aResult))
      return ErrorHandling::template convertError<OutputTy>(aResult);
    OutputTy aOutputs{};
    aOutputs[0] = ErrorHandling::unwrap(aResult);
    const auto aRest = readMany(std::forward<MoreJSONs>(theMoreJsons)...);
    if (ErrorHandling::isError(aRest))
      return ErrorHandling::template convertError<OutputTy>(aRest);
    ptrdiff_t aPos = 1;
    for (const auto &aSchema : ErrorHandling::unwrap(aRest)) {
      aOutputs[aPos++] = aSchema;
    }
    return aOutputs;
  }
  constexpr ErrorOrMany<> readMany() { return std::array<SchemaRef, 0>{}; }

  template <typename JSON, typename Section, typename... MoreSections>
  constexpr std::enable_if_t<sizeof...(MoreSections) != 0, ErrorOrConsumed>
  readSections(SchemaObject &theSchema, const std::string_view &theKey,
               const JSON &theValue) {
    const auto aErrorOrConsumed =
        Section::readSchema(*this, theSchema, theKey, theValue);
    if (ErrorHandling::isError(aErrorOrConsumed))
      return aErrorOrConsumed;
    if (ErrorHandling::unwrap(aErrorOrConsumed))
      return aErrorOrConsumed;
    return readSections<JSON, MoreSections...>(theSchema, theKey, theValue);
  }

  template <typename JSON, typename Section>
  constexpr ErrorOrConsumed readSections(SchemaObject &theSchema,
                                         const std::string_view &theKey,
                                         const JSON &theValue) {
    return Section::readSchema(*this, theSchema, theKey, theValue);
  }

  SchemaContext itsContext;
  SchemaAllocator itsSchemaAlloc;
  JsonAllocator itsJsonAlloc;
};

template <typename SchemaContext, typename ErrorHandling>
using SchemaReader =
    SchemaReaderBase<SchemaContext, ErrorHandling,
                     ReaderApplicator::SchemaReader, ReaderCore::SchemaReader,
                     ReaderFormat::SchemaReader, ReaderMetadata::SchemaReader,
                     ReaderValidation::SchemaReader,
                     ReaderContent::SchemaReader, ReaderCompat::SchemaReader>;

} // namespace json_schema
#endif // JSON_SCHEMA_SCHEMA_READER_H
