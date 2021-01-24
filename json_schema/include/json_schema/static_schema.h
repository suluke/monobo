#ifndef JSON_SCHEMA_STATIC_SCHEMA_H
#define JSON_SCHEMA_STATIC_SCHEMA_H

#include "constexpr_json/static_document.h"
#include "json_schema/schema_info.h"
#include "json_schema/schema_object.h"
#include "json_schema/util.h"

namespace json_schema {

struct StaticStorage {
  template <typename T> struct Buffer {
    size_t itsPos{0};
    size_t itsSize{0};

    constexpr size_t size() const noexcept { return itsSize; }
    constexpr bool operator==(const Buffer &theOther) const {
      return itsPos == theOther.itsPos && itsSize == theOther.itsSize;
    }
    constexpr bool operator!=(const Buffer &theOther) const {
      return !(*this == theOther);
    }
  };

  template <typename Key, typename Value>
  using MapEntry = std::pair<Key, Value>;

  template <typename Key, typename Value>
  using Map = Buffer<MapEntry<Key, Value>>;

  struct String {
    size_t itsPos{0};
    size_t itsSize{0};

    constexpr size_t size() const noexcept { return itsSize; }
  };
  struct Schema {
    ptrdiff_t itsPos{0};

    constexpr bool operator==(const Schema &theOther) const {
      return itsPos == theOther.itsPos;
    }
    constexpr bool operator!=(const Schema &theOther) const {
      return itsPos != theOther.itsPos;
    }
  };

  struct Json {
    ptrdiff_t itsPos{-1};
  };
};

template <typename Standard, size_t MAX_SCHEMAS, size_t MAX_VOCAB_ENTRIES,
          size_t MAX_SCHEMA_DICT_ENTRIES, size_t MAX_STRINGLIST_DICT_ENTRIES,
          size_t MAX_SCHEMA_LIST_ITEMS, size_t MAX_TYPES_LIST_ITEMS,
          size_t MAX_STRING_LIST_ITEMS, size_t MAX_CHARS,
          size_t MAX_JSON_LIST_ITEMS, size_t MAX_JSON_REFS,
          size_t MAX_JSON_NUMEBRS, size_t MAX_JSON_CHARS,
          size_t MAX_JSON_STRINGS, size_t MAX_JSON_ARRAYS,
          size_t MAX_JSON_ARRAY_ENTRIES, size_t MAX_JSON_OBJECTS,
          size_t MAX_JSON_OBJECT_PROPS>
class StaticSchemaContext {
public:
  constexpr StaticSchemaContext() = default;
  constexpr StaticSchemaContext(const StaticSchemaContext &) = default;
  constexpr StaticSchemaContext(StaticSchemaContext &&) = default;
  StaticSchemaContext &operator=(const StaticSchemaContext &) = default;
  StaticSchemaContext &operator=(StaticSchemaContext &&) = default;

  template <typename T, size_t N> using Buffer = std::array<T, N>;
  template <typename Key, typename Value, size_t Size>
  using Map = Buffer<std::pair<Key, Value>, Size>;
  using JsonStorage =
      cjson::StaticDocument<MAX_JSON_NUMEBRS, MAX_JSON_CHARS, MAX_JSON_STRINGS,
                            MAX_JSON_ARRAYS, MAX_JSON_ARRAY_ENTRIES,
                            MAX_JSON_OBJECTS, MAX_JSON_OBJECT_PROPS>;

  using Storage = StaticStorage;
  using SchemaRef = typename Storage::Schema;
  using SchemaObject = typename Standard::template SchemaObject<Storage>;
  using JsonRef = typename Storage::Json;
  using StringRef = typename Storage::String;
  template <typename T> using BufferRef = typename Storage::template Buffer<T>;
  template <typename Key, typename Value>
  using MapEntry = std::pair<Key, Value>;
  template <typename KeyT, typename ValT>
  using MapRef = typename Storage::template Map<KeyT, ValT>;

  template <typename ErrorHandling>
  struct Allocator : SchemaInfo,
                     cjson::DocumentAllocator<JsonStorage, ErrorHandling> {
    using JsonAlloc = cjson::DocumentAllocator<JsonStorage, ErrorHandling>;
    using String = typename Storage::String;
    template <typename T> using Buffer = typename Storage::template Buffer<T>;
    template <typename KeyT, typename ValT>
    using Map = typename Storage::template Map<KeyT, ValT>;

    constexpr Allocator() = default;

    constexpr SchemaRef allocateSchema(const SchemaObject &theSchema,
                                       StaticSchemaContext &theContext) {
      theContext.itsSchemaObjects[NUM_SCHEMA_OBJECTS] = theSchema;
      return SchemaRef{static_cast<ptrdiff_t>(NUM_SCHEMA_OBJECTS++)};
    }
    constexpr String allocateString(StaticSchemaContext &theContext,
                                    const std::string_view &theStr) {
      String aStr{NUM_CHARS, theStr.size()};
      for (const char aChar : theStr) {
        theContext.itsChars[NUM_CHARS++] = aChar;
      }
      return aStr;
    }

    constexpr Buffer<SchemaRef> allocateBuffer(StaticSchemaContext &theContext,
                                               const size_t theSize,
                                               const type_tag<SchemaRef>) {
      Buffer<SchemaRef> aResult{NUM_SCHEMA_LIST_ITEMS, theSize};
      NUM_SCHEMA_LIST_ITEMS += theSize;
      if (NUM_SCHEMA_LIST_ITEMS > MAX_SCHEMA_LIST_ITEMS)
        throw "Over-allocating schema buffers";
      return aResult;
    }
    constexpr Buffer<String> allocateBuffer(StaticSchemaContext &theContext,
                                            const size_t theSize,
                                            const type_tag<String>) {
      Buffer<String> aResult{NUM_STRING_LIST_ITEMS, theSize};
      NUM_STRING_LIST_ITEMS += theSize;
      if (NUM_STRING_LIST_ITEMS > MAX_STRING_LIST_ITEMS)
        throw "Over-allocating string buffers";
      return aResult;
    }
    constexpr Buffer<Types> allocateBuffer(StaticSchemaContext &theContext,
                                           const size_t theSize,
                                           const type_tag<Types>) {
      Buffer<Types> aResult{NUM_TYPES_LIST_ITEMS, theSize};
      NUM_TYPES_LIST_ITEMS += theSize;
      if (NUM_TYPES_LIST_ITEMS > MAX_TYPES_LIST_ITEMS)
        throw "Over-allocating types buffers";
      return aResult;
    }
    constexpr Buffer<JsonRef> allocateBuffer(StaticSchemaContext &theContext,
                                             const size_t theSize,
                                             const type_tag<JsonRef>) {
      Buffer<JsonRef> aResult{NUM_JSON_LIST_ITEMS, theSize};
      NUM_JSON_LIST_ITEMS += theSize;
      if (NUM_JSON_LIST_ITEMS > MAX_JSON_LIST_ITEMS)
        throw "Over-allocating json buffers";
      return aResult;
    }
    constexpr Map<String, bool> allocateMap(StaticSchemaContext &theContext,
                                            const size_t theSize,
                                            const type_tag<String, bool>) {
      Map<String, bool> aMap{NUM_VOCAB_ENTRIES, theSize};
      NUM_VOCAB_ENTRIES += theSize;
      return aMap;
    }
    constexpr Map<String, SchemaRef>
    allocateMap(StaticSchemaContext &theContext, const size_t theSize,
                const type_tag<String, SchemaRef>) {
      Map<String, SchemaRef> aMap{NUM_SCHEMA_DICT_ENTRIES, theSize};
      NUM_SCHEMA_DICT_ENTRIES += theSize;
      return aMap;
    }
    constexpr Map<String, BufferRef<StringRef>>
    allocateMap(StaticSchemaContext &theContext, const size_t theSize,
                const type_tag<String, BufferRef<StringRef>>) {
      Map<String, BufferRef<StringRef>> aMap{NUM_STRINGLIST_DICT_ENTRIES,
                                             theSize};
      NUM_STRINGLIST_DICT_ENTRIES += theSize;
      return aMap;
    }

    template <typename JSON>
    constexpr JsonRef allocateJson(StaticSchemaContext &theContext,
                                   const JSON &theJson) {
      const cjson::Entity aEntity =
          JsonAlloc::allocateJson(theContext.itsJsonStorage, theJson);
      theContext.itsJsons[itsJsons] = aEntity;
      return {itsJsons++};
    }

  private:
    ptrdiff_t itsJsons{0};
  };

  constexpr SchemaRef getTrueSchemaRef() const { return SchemaRef{-1}; }
  constexpr const SchemaObject &getSchemaObject(const SchemaRef &theRef) const {
    return itsSchemaObjects[theRef.itsPos];
  }

  /// Map in-the-wild Storage::Buffer objects to accesses on this context's
  /// internal buffers
  template <typename T, bool IsConst = true> class BufferAccessor {
  public:
    using ContextTy = std::conditional_t<IsConst, const StaticSchemaContext,
                                         StaticSchemaContext>;
    using Value = std::conditional_t<IsConst, const T, T>;
    constexpr BufferAccessor(ContextTy &theContext, BufferRef<T> theBuffer)
        : itsContext(&theContext), itsBuffer(theBuffer) {}

    constexpr size_t size() const { return itsBuffer.size(); }
    constexpr auto operator[](const ptrdiff_t theIdx) const {
      return prettify(access(*itsContext, itsBuffer, theIdx));
    }

    constexpr Value &getRawRef(const ptrdiff_t theIdx) const {
      return access(*itsContext, itsBuffer, theIdx);
    }

    constexpr bool operator==(const BufferAccessor &theOther) const {
      return itsContext == theOther.itsContext &&
             itsBuffer == theOther.itsBuffer;
    }
    constexpr bool operator!=(const BufferAccessor &theOther) const {
      return !(*this == theOther);
    }

    class Iterator {
    public:
      constexpr Iterator() = default;
      constexpr Iterator(const Iterator &) = default;
      constexpr Iterator &operator=(const Iterator &) = default;

      constexpr Iterator(const BufferAccessor &theBuf, const ptrdiff_t thePos)
          : itsPos{thePos}, itsBuffer{theBuf} {}

      constexpr Iterator &operator++() {
        ++itsPos;
        return *this;
      }
      constexpr Iterator operator++(int) {
        Iterator aCopy{*this};
        ++(*this);
        return aCopy;
      }
      constexpr auto operator*() const { return itsBuffer[itsPos]; }
      constexpr bool operator==(const Iterator &theOther) const {
        return itsPos == theOther.itsPos && itsBuffer == theOther.itsBuffer;
      }
      constexpr bool operator!=(const Iterator &theOther) const {
        return !(*this == theOther);
      }

    private:
      ptrdiff_t itsPos;
      BufferAccessor itsBuffer;
    };
    constexpr Iterator begin() const { return {*this, 0}; }
    constexpr Iterator end() const {
      return {*this, static_cast<ptrdiff_t>(itsBuffer.size())};
    }

  private:
    ContextTy *itsContext;
    BufferRef<T> itsBuffer;

  protected:
    template <typename T1, typename T2>
    constexpr auto prettify(const std::pair<T1, T2> &thePair) const {
      return std::make_pair(prettify(thePair.first), prettify(thePair.second));
    }
    constexpr bool prettify(const bool theBool) const { return theBool; }
    constexpr std::string_view prettify(const StringRef theString) const {
      return itsContext->getString(theString);
    }
    constexpr auto prettify(const SchemaRef theSchema) const {
      using ReturnTy = std::variant<bool, SchemaObjectAccessor<StaticSchemaContext>>;
      if (itsContext->getTrueSchemaRef() == theSchema)
        return ReturnTy{true};
      return ReturnTy{SchemaObjectAccessor<StaticSchemaContext>{*itsContext, theSchema}};
    }

  private:
    static constexpr Value &access(ContextTy &theCtx,
                                   const BufferRef<SchemaRef> theBuf,
                                   const size_t theIdx) {
      return theCtx.itsSchemaLists[theBuf.itsPos + theIdx];
    }
    static constexpr Value &access(ContextTy &theCtx,
                                   const BufferRef<StringRef> theBuf,
                                   const size_t theIdx) {
      return theCtx.itsStringLists[theBuf.itsPos + theIdx];
    }
    static constexpr Value &access(ContextTy &theCtx,
                                   const BufferRef<Types> theBuf,
                                   const size_t theIdx) {
      return theCtx.itsTypesLists[theBuf.itsPos + theIdx];
    }
    static constexpr Value &access(ContextTy &theCtx,
                                   const BufferRef<JsonRef> theBuf,
                                   const size_t theIdx) {
      return theCtx.itsJsonLists[theBuf.itsPos + theIdx];
    }
    static constexpr Value &access(ContextTy &theCtx,
                                   const MapRef<StringRef, bool> theBuf,
                                   const size_t theIdx) {
      return theCtx.itsVocabEntries[theBuf.itsPos + theIdx];
    }
    static constexpr Value &access(ContextTy &theCtx,
                                   const MapRef<StringRef, SchemaRef> theBuf,
                                   const size_t theIdx) {
      return theCtx.itsStringSchemaMaps[theBuf.itsPos + theIdx];
    }
    static constexpr Value &
    access(ContextTy &theCtx,
           const MapRef<StringRef, BufferRef<StringRef>> theBuf,
           const size_t theIdx) {
      return theCtx.itsStringListDicts[theBuf.itsPos + theIdx];
    }
  };
  template <typename KeyT, typename ValT>
  struct MapAccessor : BufferAccessor<std::pair<KeyT, ValT>> {
    using Base = BufferAccessor<std::pair<KeyT, ValT>>;
    using ContextTy = typename Base::ContextTy;
    using Base::begin;
    using Base::end;
    using Base::operator[];
    using Base::operator==;
    using Base::operator!=;
    using Base::size;

    constexpr MapAccessor(ContextTy &theContext, MapRef<KeyT, ValT> theMap)
        : Base(theContext, theMap) {}

  public:
    constexpr auto operator[](const std::string_view &theKey) const {
      using LookupResult =
          std::optional<decltype(Base::prettify(std::declval<const ValT &>()))>;

      for (const auto [aKey, aVal] : *this) {
        if (aKey == theKey)
          return LookupResult{Base::prettify(aVal)};
      }
      return LookupResult{std::nullopt};
    }
  };

  template <typename T>
  constexpr void setBufferItem(BufferRef<T> &theList, const ptrdiff_t theIdx,
                               const T &theValue) {
    BufferAccessor<T, false>{*this, theList}.getRawRef(theIdx) = theValue;
  }
  template <typename KeyT, typename ValT>
  constexpr void setMapEntry(MapRef<KeyT, ValT> &theMap, const ptrdiff_t theIdx,
                             const KeyT &theKey, const ValT &theVal) {
    BufferAccessor<MapEntry<KeyT, ValT>, false> aMapRef{*this, theMap};
    aMapRef.getRawRef(theIdx).first = theKey;
    aMapRef.getRawRef(theIdx).second = theVal;
  }

  constexpr std::string_view getString(const StringRef &theStr) const {
    return {itsChars.data() + theStr.itsPos, theStr.itsSize};
  }

private:
  template <typename ErrorHandling> friend struct Allocator;
  friend SchemaRef;

  template <typename, bool> friend class BufferAccessor;

  Buffer<SchemaObject, MAX_SCHEMAS> itsSchemaObjects{};
  Buffer<typename Vocabulary<Storage>::Entry, MAX_VOCAB_ENTRIES>
      itsVocabEntries{};
  Buffer<typename Defs<Storage>::Entry, MAX_SCHEMA_DICT_ENTRIES>
      itsStringSchemaMaps{};
  Buffer<SchemaRef, MAX_SCHEMA_LIST_ITEMS> itsSchemaLists{};
  Buffer<Types, MAX_TYPES_LIST_ITEMS> itsTypesLists{};
  Buffer<StringRef, MAX_STRING_LIST_ITEMS> itsStringLists{};
  Buffer<char, MAX_CHARS> itsChars{};
  Buffer<MapEntry<StringRef, BufferRef<StringRef>>, MAX_STRINGLIST_DICT_ENTRIES>
      itsStringListDicts{};
  Buffer<JsonRef, MAX_JSON_LIST_ITEMS> itsJsonLists{};
  Buffer<cjson::Entity, MAX_JSON_REFS> itsJsons;
  JsonStorage itsJsonStorage{cjson::DocumentInfo{
      0 /*=numNulls*/, 0 /*=numBools*/, MAX_JSON_NUMEBRS, MAX_JSON_CHARS,
      MAX_JSON_STRINGS, MAX_JSON_ARRAYS, MAX_JSON_ARRAY_ENTRIES,
      MAX_JSON_OBJECTS, MAX_JSON_OBJECT_PROPS}};
};

#define JSON_SCHEMA_STATIC_CONTEXT_TYPE(theStandard, theInfo)                  \
  json_schema::StaticSchemaContext<                                            \
      theStandard, (theInfo).NUM_SCHEMA_OBJECTS, (theInfo).NUM_VOCAB_ENTRIES,  \
      (theInfo).NUM_SCHEMA_DICT_ENTRIES,                                       \
      (theInfo).NUM_STRINGLIST_DICT_ENTRIES, (theInfo).NUM_SCHEMA_LIST_ITEMS,  \
      (theInfo).NUM_TYPES_LIST_ITEMS, (theInfo).NUM_STRING_LIST_ITEMS,         \
      (theInfo).NUM_CHARS, (theInfo).NUM_JSON_LIST_ITEMS,                      \
      (theInfo).NUM_JSON_REFS, (theInfo).JSON_INFO.itsNumNumbers,              \
      (theInfo).JSON_INFO.itsNumChars, (theInfo).JSON_INFO.itsNumStrings,      \
      (theInfo).JSON_INFO.itsNumArrays,                                        \
      (theInfo).JSON_INFO.itsNumArrayEntries,                                  \
      (theInfo).JSON_INFO.itsNumObjects,                                       \
      (theInfo).JSON_INFO.itsNumObjectProperties>

} // namespace json_schema
#endif // JSON_SCHEMA_STATIC_SCHEMA_H
