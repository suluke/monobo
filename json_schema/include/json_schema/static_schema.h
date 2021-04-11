#ifndef JSON_SCHEMA_STATIC_SCHEMA_H
#define JSON_SCHEMA_STATIC_SCHEMA_H

#include "constexpr_json/static_document.h"
#include "json_schema/schema_info.h"
#include "json_schema/schema_object.h"
#include "json_schema/util.h"

namespace json_schema {

struct StaticStorage {
  template <typename T> struct Ptr : public json_schema::Optional<T> {
    using element_type = const T;
    static constexpr Ptr pointer_to(element_type &theRef) noexcept {
      return Ptr{theRef};
    }
  };

  template <typename T> constexpr static auto pointer_to(T &&theRef) {
    return pointer_traits<Ptr<std::remove_reference_t<T>>>::pointer_to(theRef);
  }

  template <typename T> struct BufferRef {
    using Self = BufferRef;
    ptrdiff_t itsPos{0};
    size_t itsSize{0};
    size_t itsCapacity{0};

    constexpr size_t size() const noexcept { return itsSize; }
    constexpr bool operator==(const Self &theOther) const {
      return itsPos == theOther.itsPos && itsSize == theOther.itsSize;
    }
    constexpr bool operator!=(const Self &theOther) const {
      return !(*this == theOther);
    }
  };
  template <typename T> using BufferPtr = Ptr<BufferRef<T>>;

  template <typename Key, typename Value>
  using MapEntry = std::pair<Key, Value>;

  template <typename Key, typename Value>
  using MapRef = BufferRef<MapEntry<Key, Value>>;

  template <typename Key, typename Value>
  using MapPtr = BufferPtr<MapEntry<Key, Value>>;

  struct StringRef {
    ptrdiff_t itsPos{0};
    size_t itsSize{0};

    constexpr size_t size() const noexcept { return itsSize; }
  };
  using StringPtr = Ptr<StringRef>;
  struct SchemaRef {
    using Self = SchemaRef;

    // SchemaRef() = delete;

    ptrdiff_t itsPos{0};

    constexpr bool operator==(const Self &theOther) const {
      return itsPos == theOther.itsPos;
    }
    constexpr bool operator!=(const Self &theOther) const {
      return itsPos != theOther.itsPos;
    }
  };
  using SchemaPtr = Ptr<SchemaRef>;

  struct JsonRef {
    ptrdiff_t itsPos{0};
  };
  using JsonPtr = Ptr<JsonRef>;
};

template <typename Standard_, size_t MAX_SCHEMAS, size_t MAX_VOCAB_ENTRIES,
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

  using Standard = Standard_;
  template <typename T, size_t N> using Buffer = std::array<T, N>;
  template <typename Key, typename Value, size_t Size>
  using Map = Buffer<std::pair<Key, Value>, Size>;
  using JsonStorage =
      cjson::StaticDocument<MAX_JSON_NUMEBRS, MAX_JSON_CHARS, MAX_JSON_STRINGS,
                            MAX_JSON_ARRAYS, MAX_JSON_ARRAY_ENTRIES,
                            MAX_JSON_OBJECTS, MAX_JSON_OBJECT_PROPS>;

  using Storage = StaticStorage;
  using SchemaRef = typename Storage::SchemaRef;
  using SchemaObject = typename Standard::template SchemaObject<Storage>;
  using JsonRef = typename Storage::JsonRef;
  using JsonAccessor = typename JsonStorage::EntityRef;
  using StringRef = typename Storage::StringRef;
  template <typename T>
  using BufferRef = typename Storage::template BufferRef<T>;
  template <typename Key, typename Value>
  using MapEntry = std::pair<Key, Value>;
  template <typename KeyT, typename ValT>
  using MapRef = typename Storage::template BufferRef<MapEntry<KeyT, ValT>>;

  template <typename ErrorHandling>
  struct Allocator : SchemaInfo,
                     cjson::DocumentAllocator<JsonStorage, ErrorHandling> {
    using JsonAlloc = cjson::DocumentAllocator<JsonStorage, ErrorHandling>;

    constexpr Allocator() = default;

    constexpr SchemaRef allocateSchema(const SchemaObject &theSchema,
                                       StaticSchemaContext &theContext) {
      theContext.itsSchemaObjects[NUM_SCHEMA_OBJECTS] = theSchema;
      return SchemaRef{static_cast<ptrdiff_t>(NUM_SCHEMA_OBJECTS++)};
    }
    constexpr StringRef allocateString(StaticSchemaContext &theContext,
                                       const std::string_view &theStr) {
      StringRef aStr{static_cast<ptrdiff_t>(NUM_CHARS), theStr.size()};
      for (const char aChar : theStr) {
        theContext.itsChars[NUM_CHARS++] = aChar;
      }
      return aStr;
    }

    constexpr BufferRef<SchemaRef>
    allocateBuffer(StaticSchemaContext &theContext, const size_t theSize,
                   const type_tag<SchemaRef>) {
      BufferRef<SchemaRef> aResult{
          static_cast<ptrdiff_t>(NUM_SCHEMA_LIST_ITEMS), 0, theSize};
      NUM_SCHEMA_LIST_ITEMS += theSize;
      if (NUM_SCHEMA_LIST_ITEMS > MAX_SCHEMA_LIST_ITEMS)
        throw "Over-allocating schema buffers";
      return aResult;
    }
    constexpr BufferRef<StringRef>
    allocateBuffer(StaticSchemaContext &theContext, const size_t theSize,
                   const type_tag<StringRef>) {
      BufferRef<StringRef> aResult{
          static_cast<ptrdiff_t>(NUM_STRING_LIST_ITEMS), 0, theSize};
      NUM_STRING_LIST_ITEMS += theSize;
      if (NUM_STRING_LIST_ITEMS > MAX_STRING_LIST_ITEMS)
        throw "Over-allocating string buffers";
      return aResult;
    }
    constexpr BufferRef<Types> allocateBuffer(StaticSchemaContext &theContext,
                                              const size_t theSize,
                                              const type_tag<Types>) {
      BufferRef<Types> aResult{static_cast<ptrdiff_t>(NUM_TYPES_LIST_ITEMS), 0,
                               theSize};
      NUM_TYPES_LIST_ITEMS += theSize;
      if (NUM_TYPES_LIST_ITEMS > MAX_TYPES_LIST_ITEMS)
        throw "Over-allocating types buffers";
      return aResult;
    }
    constexpr BufferRef<JsonRef> allocateBuffer(StaticSchemaContext &theContext,
                                                const size_t theSize,
                                                const type_tag<JsonRef>) {
      BufferRef<JsonRef> aResult{static_cast<ptrdiff_t>(NUM_JSON_LIST_ITEMS), 0,
                                 theSize};
      NUM_JSON_LIST_ITEMS += theSize;
      if (NUM_JSON_LIST_ITEMS > MAX_JSON_LIST_ITEMS)
        throw "Over-allocating json buffers";
      return aResult;
    }
    constexpr MapRef<StringRef, bool>
    allocateMap(StaticSchemaContext &theContext, const size_t theSize,
                const type_tag<StringRef, bool>) {
      MapRef<StringRef, bool> aMap{static_cast<ptrdiff_t>(NUM_VOCAB_ENTRIES), 0,
                                   theSize};
      NUM_VOCAB_ENTRIES += theSize;
      return aMap;
    }
    constexpr MapRef<StringRef, SchemaRef>
    allocateMap(StaticSchemaContext &theContext, const size_t theSize,
                const type_tag<StringRef, SchemaRef>) {
      MapRef<StringRef, SchemaRef> aMap{
          static_cast<ptrdiff_t>(NUM_SCHEMA_DICT_ENTRIES), 0, theSize};
      NUM_SCHEMA_DICT_ENTRIES += theSize;
      return aMap;
    }
    constexpr MapRef<StringRef, BufferRef<StringRef>>
    allocateMap(StaticSchemaContext &theContext, const size_t theSize,
                const type_tag<StringRef, BufferRef<StringRef>>) {
      MapRef<StringRef, BufferRef<StringRef>> aMap{
          static_cast<ptrdiff_t>(NUM_STRINGLIST_DICT_ENTRIES), 0, theSize};
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

  constexpr const SchemaObject &getSchemaObject(const SchemaRef &theRef) const {
    return itsSchemaObjects[theRef.itsPos];
  }
  constexpr bool isTrueSchema(const SchemaRef &theRef) const noexcept {
    return theRef.itsPos == 0;
  }
  constexpr bool isFalseSchema(const SchemaRef &theRef) const noexcept {
    return theRef.itsPos == 1;
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
      return prettify(getStorage()[itsBuffer.itsPos + theIdx]);
    }

    constexpr Value &getRawRef(const ptrdiff_t theIdx) const {
      return getStorage()[theIdx];
    }
    constexpr auto &getStorage() const {
      return getStorage(*itsContext, type_tag<T>{});
    }

    constexpr bool operator==(const BufferAccessor &theOther) const {
      return itsContext == theOther.itsContext &&
             itsBuffer == theOther.itsBuffer;
    }
    constexpr bool operator!=(const BufferAccessor &theOther) const {
      return !(*this == theOther);
    }

    using Iterator = SubscriptIterator<BufferAccessor>;
    constexpr Iterator begin() const { return {*this, 0}; }
    constexpr Iterator end() const {
      return {*this, static_cast<ptrdiff_t>(itsBuffer.size())};
    }

  private:
    ContextTy *itsContext;
    BufferRef<T> itsBuffer;

  protected:
    template <typename T2>
    constexpr auto prettify(const BufferRef<T2> &theBuffer) const {
      return BufferAccessor<T2, IsConst>{*itsContext, theBuffer};
    }
    template <typename T1, typename T2>
    constexpr auto prettify(const std::pair<T1, T2> &thePair) const {
      return std::make_pair(prettify(thePair.first), prettify(thePair.second));
    }
    constexpr bool prettify(const bool theBool) const { return theBool; }
    constexpr Types prettify(const Types theType) const { return theType; }
    constexpr std::string_view prettify(const StringRef theString) const {
      return itsContext->getString(theString);
    }
    constexpr auto prettify(const JsonRef theJson) const {
      return itsContext->getJson(theJson);
    }
    constexpr auto prettify(const SchemaRef theSchema) const {
      return SchemaObjectAccessor<StaticSchemaContext>{*itsContext, theSchema};
    }

  private:
    static constexpr inline auto &getStorage(ContextTy &theCtx,
                                             const type_tag<SchemaRef>) {
      return theCtx.itsSchemaLists;
    }
    static constexpr inline auto &getStorage(ContextTy &theCtx,
                                             const type_tag<StringRef>) {
      return theCtx.itsStringLists;
    }
    static constexpr inline auto &getStorage(ContextTy &theCtx,
                                             const type_tag<Types>) {
      return theCtx.itsTypesLists;
    }
    static constexpr inline auto &getStorage(ContextTy &theCtx,
                                             const type_tag<JsonRef>) {
      return theCtx.itsJsonLists;
    }
    static constexpr inline auto &
    getStorage(ContextTy &theCtx, const type_tag<MapEntry<StringRef, bool>>) {
      return theCtx.itsVocabEntries;
    }
    static constexpr inline auto &
    getStorage(ContextTy &theCtx,
               const type_tag<MapEntry<StringRef, SchemaRef>>) {
      return theCtx.itsStringSchemaMaps;
    }
    static constexpr inline auto &
    getStorage(ContextTy &theCtx,
               const type_tag<MapEntry<StringRef, BufferRef<StringRef>>>) {
      return theCtx.itsStringListDicts;
    }
  };
  template <typename KeyT, typename ValT>
  struct MapAccessor : BufferAccessor<std::pair<KeyT, ValT>> {
    using Base = BufferAccessor<std::pair<KeyT, ValT>>;
    using ContextTy = typename Base::ContextTy;
    using Base::begin;
    using Base::end;
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
  constexpr void extendBuffer(BufferRef<T> &theList, const T &theValue) {
    if (theList.itsSize >= theList.itsCapacity)
      throw "Exceeding buffer capacity";
    BufferAccessor<T, false>{*this, theList}.getStorage()[theList.itsSize] =
        theValue;
    ++theList.itsSize;
  }
  template <typename KeyT, typename ValT>
  constexpr void addMapEntry(MapRef<KeyT, ValT> &theMap, const KeyT &theKey,
                             const ValT &theVal) {
    if (theMap.itsSize >= theMap.itsCapacity)
      throw "Exceeding map capacity";
    BufferAccessor<MapEntry<KeyT, ValT>, false> aMapRef{*this, theMap};
    aMapRef.getStorage()[theMap.itsSize].first = theKey;
    aMapRef.getStorage()[theMap.itsSize].second = theVal;
    ++theMap.itsSize;
  }

  constexpr std::string_view getString(const StringRef &theStr) const {
    return {itsChars.data() + theStr.itsPos, theStr.itsSize};
  }
  constexpr JsonAccessor getJson(const JsonRef &theJson) const {
    return JsonAccessor{itsJsonStorage, itsJsons[theJson.itsPos]};
  }

private:
  template <typename, bool> friend class BufferAccessor;

  Buffer<SchemaObject, MAX_SCHEMAS> itsSchemaObjects{};
  Buffer<MapEntry<StringRef, bool>, MAX_VOCAB_ENTRIES> itsVocabEntries{};
  Buffer<MapEntry<StringRef, SchemaRef>, MAX_SCHEMA_DICT_ENTRIES>
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
      theStandard,                                                             \
      (theInfo).NUM_SCHEMA_OBJECTS + 2 /* Always count `true` and `false`*/,   \
      (theInfo).NUM_VOCAB_ENTRIES, (theInfo).NUM_SCHEMA_DICT_ENTRIES,          \
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
