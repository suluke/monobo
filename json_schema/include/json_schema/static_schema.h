#ifndef JSON_SCHEMA_STATIC_SCHEMA_H
#define JSON_SCHEMA_STATIC_SCHEMA_H

#include "constexpr_json/static_document.h"
#include "json_schema/model/schema_object.h"
#include "json_schema/schema_info.h"
#include "json_schema/util.h"

namespace json_schema {
template <size_t MAX_SCHEMAS, size_t MAX_VOCAB_ENTRIES,
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

  struct Storage {
    template <typename T> struct Buffer {
      size_t itsPos{0};
      size_t itsSize{0};
    };

    template <typename Key, typename Value>
    using MapEntry = std::pair<Key, Value>;

    template <typename Key, typename Value>
    using Map = Buffer<MapEntry<Key, Value>>;

    struct String {
      size_t itsPos{0};
      size_t itsSize{0};
    };
    struct Schema {
      ptrdiff_t itsPos{0};
    };

    struct Json {
      ptrdiff_t itsPos{-1};
    };
  };

  using SchemaRef = typename Storage::Schema;
  using SchemaObject = json_schema::SchemaObject<Storage>;
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

    constexpr Allocator() { NUM_SCHEMAS = 1; }

    constexpr SchemaRef allocateSchema(const SchemaObject &theSchema,
                                       StaticSchemaContext &theContext) {
      theContext.itsSchemaObjects[NUM_SCHEMAS] = theSchema;
      return SchemaRef{static_cast<ptrdiff_t>(NUM_SCHEMAS++)};
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

  constexpr void setBufferItem(BufferRef<SchemaRef> &theList,
                               const ptrdiff_t theIdx,
                               const SchemaRef &theValue) {
    itsSchemaLists[theList.itsPos + theIdx] = theValue;
  }
  constexpr void setBufferItem(BufferRef<StringRef> &theList,
                               const ptrdiff_t theIdx,
                               const StringRef &theValue) {
    itsStringLists[theList.itsPos + theIdx] = theValue;
  }
  constexpr void setBufferItem(BufferRef<Types> &theList,
                               const ptrdiff_t theIdx, const Types theValue) {
    itsTypesLists[theList.itsPos + theIdx] = theValue;
  }
  constexpr void setBufferItem(BufferRef<JsonRef> &theList,
                               const ptrdiff_t theIdx, const JsonRef theValue) {
    itsJsonLists[theList.itsPos + theIdx] = theValue;
  }
  constexpr void setMapEntry(MapRef<StringRef, bool> &theMap,
                             const ptrdiff_t theIdx, const StringRef &theKey,
                             const bool &theVal) {
    itsVocabEntries[theMap.itsPos + theIdx].first = theKey;
    itsVocabEntries[theMap.itsPos + theIdx].second = theVal;
  }
  constexpr void setMapEntry(MapRef<StringRef, SchemaRef> &theMap,
                             const ptrdiff_t theIdx, const StringRef &theKey,
                             const SchemaRef &theVal) {
    itsStringSchemaMaps[theMap.itsPos + theIdx].first = theKey;
    itsStringSchemaMaps[theMap.itsPos + theIdx].second = theVal;
  }
  constexpr void setMapEntry(MapRef<StringRef, BufferRef<StringRef>> &theMap,
                             const ptrdiff_t theIdx, const StringRef &theKey,
                             const BufferRef<StringRef> &theVal) {
    itsStringListDicts[theMap.itsPos + theIdx].first = theKey;
    itsStringListDicts[theMap.itsPos + theIdx].second = theVal;
  }

  constexpr std::string_view getString(const StringRef &theStr) const {
    return {itsChars.data() + theStr.itsPos, theStr.itsSize};
  }

private:
  template <typename ErrorHandling> friend struct Allocator;
  friend SchemaRef;

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

#define JSON_SCHEMA_STATIC_CONTEXT_TYPE(theInfo)                               \
  json_schema::StaticSchemaContext<                                            \
      (theInfo).NUM_SCHEMAS, (theInfo).NUM_VOCAB_ENTRIES,                      \
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
