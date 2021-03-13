#ifndef JSON_SCHEMA_SCHEMA_READER_H
#define JSON_SCHEMA_SCHEMA_READER_H

#include "constexpr_json/impl/document_allocator.h"
#include "json_schema/util.h"

#include <variant>

namespace json_schema {
template <bool LENIENT, typename SchemaContext, typename ErrorHandling_, typename... Sections>
class SchemaReaderBase {
public:
  using ErrorHandling = ErrorHandling_;
  using Storage = typename SchemaContext::Storage;
  using StringRef = typename Storage::StringRef;
  using SchemaRef = typename Storage::SchemaRef;
  using SchemaObject = typename SchemaContext::SchemaObject;
  using ErrorOrConsumed = typename ErrorHandling::template ErrorOr<bool>;
  using SchemaAllocator =
      typename SchemaContext::template Allocator<ErrorHandling>;

  // FIXME should be renamed to ListRef
  template <typename T> using List = typename Storage::template BufferRef<T>;
  template <typename KeyT, typename ValT>
  using MapRef = typename Storage::template MapRef<KeyT, ValT>;

  template <typename... JSONs> struct ReadResult {
    using SchemaObject = SchemaObjectAccessor<SchemaContext>;
    SchemaContext itsContext;
    std::array<SchemaRef, sizeof...(JSONs)> itsSchemas;

    constexpr std::variant<bool, const SchemaObject>
    operator[](const ptrdiff_t theIdx) const {
      if (itsContext.getTrueSchemaRef() == itsSchemas[theIdx])
        return true;
      return SchemaObject{itsContext, itsSchemas[theIdx]};
    }
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

  template <typename T> static constexpr auto toPtr(T &&theRef) {
    return SchemaContext::Storage::pointer_to(std::forward<T>(theRef));
  }

  constexpr StringRef
  allocateString(const std::string_view &theStr) {
    return itsSchemaAlloc.allocateString(itsContext, theStr);
  }

  template <typename JSON>
  constexpr typename Storage::JsonRef allocateJson(const JSON &theJson) {
    return itsSchemaAlloc.allocateJson(itsContext, theJson);
  }

  template <typename T> constexpr List<T> allocateList(const size_t theSize) {
    return itsSchemaAlloc.allocateBuffer(itsContext, theSize, type_tag<T>{});
  }

  template <typename T>
  constexpr void appendToList(List<T> &theBuf,
                             const T &theVal) {
    itsContext.extendBuffer(theBuf, theVal);
  }

  template <typename KeyT, typename ValT>
  constexpr MapRef<KeyT, ValT> allocateMap(const size_t theSize) {
    return itsSchemaAlloc.allocateMap(itsContext, theSize,
                                      type_tag<KeyT, ValT>{});
  }

  template <typename KeyT, typename ValT>
  constexpr void addMapEntry(MapRef<KeyT, ValT> &theMap,
                             const KeyT &theKey, const ValT &theVal) {
    itsContext.addMapEntry(theMap, theKey, theVal);
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
        if (!ErrorHandling::unwrap(aErrorOrConsumed) && !LENIENT)
          return ErrorHandling::template makeError<SchemaRef>(
              "Encountered unknown entity in schema");
      }
      return itsSchemaAlloc.allocateSchema(aSchema, itsContext);
    }
    return ErrorHandling::template makeError<SchemaRef>(
        "Schema is neither of type bool nor object");
  }

  template <typename JSON>
  constexpr auto readSchemaList(const JSON &theJson) ->
      typename ErrorHandling::template ErrorOr<List<SchemaRef>> {
    using ListRefTy = List<SchemaRef>;
    ListRefTy aSchemaBuf = allocateList<SchemaRef>(theJson.toArray().size());
    for (const auto &aJsonItem : theJson.toArray()) {
      const auto aSchema = readSchema(aJsonItem);
      if (ErrorHandling::isError(aSchema))
        return ErrorHandling::template convertError<ListRefTy>(aSchema);
      appendToList(aSchemaBuf, ErrorHandling::unwrap(aSchema));
    }
    return aSchemaBuf;
  }

  template <typename JSON>
  constexpr auto readSchemaDict(const JSON &theJson) ->
      typename ErrorHandling::template ErrorOr<
          MapRef<StringRef, SchemaRef>> {
    using MapRefTy = MapRef<StringRef, SchemaRef>;
    MapRefTy aSchemaDict = allocateMap<StringRef, SchemaRef>(
        theJson.toObject().size());
    for (const auto &[aKey, aValue] : theJson.toObject()) {
      const auto aStr = allocateString(aKey);
      const SchemaRef aSchema = readSchema(aValue);
      if (ErrorHandling::isError(aSchema))
        return ErrorHandling::template convertError<MapRefTy>(aSchema);
      addMapEntry(aSchemaDict, aStr, ErrorHandling::unwrap(aSchema));
    }
    return aSchemaDict;
  }

  template <typename JSON>
  constexpr auto readStringList(const JSON &theJson) ->
      typename ErrorHandling::template ErrorOr<
          List<StringRef>> {
    List<StringRef> aStringBuf =
        allocateList<StringRef>(theJson.toArray().size());
    for (const auto &aJsonItem : theJson.toArray())
      appendToList(aStringBuf, allocateString(aJsonItem.toString()));
    return aStringBuf;
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
};
} // namespace json_schema
#endif // JSON_SCHEMA_SCHEMA_READER_H
