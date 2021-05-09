#ifndef JSON_SCHEMA_DYNAMIC_SCHEMA_H
#define JSON_SCHEMA_DYNAMIC_SCHEMA_H

#include "constexpr_json/dynamic_document.h"
#include "constexpr_json/ext/error_is_except.h"
#include "constexpr_json/impl/document_allocator.h"
#include "json_schema/schema_object.h"
#include "json_schema/util.h"
#include <any>
#include <map>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace json_schema {
struct DynamicStorage {
  template <typename T> struct Ref : public std::reference_wrapper<T> {
    Ref() = delete;
    Ref(T &theRef) : std::reference_wrapper<T>(theRef) {}
    Ref &operator=(T &theRef) {
      this->std::reference_wrapper<T>::operator=(theRef);
      return *this;
    }

    bool operator<(const Ref &theOther) const {
      return this->get() < theOther.get();
    }
    bool operator==(const Ref &theOther) const {
      return this->get() == theOther.get();
    }
    bool operator!=(const Ref &theOther) const {
      return this->get() != theOther.get();
    }
  };
  template <typename T> using Ptr = T *;
  template <typename T> using BufferRef = Ref<std::vector<T>>;
  template <typename T> using BufferPtr = std::vector<T> *;
  template <typename Key, typename Value> using Map = std::map<Key, Value>;
  template <typename Key, typename Value> using MapRef = Ref<Map<Key, Value>>;
  template <typename Key, typename Value> using MapPtr = Ptr<Map<Key, Value>>;
  template <typename Key, typename Value>
  using MapEntry = typename std::decay_t<MapRef<Key, Value>>::value_type;
  using StringRef = Ref<std::string>;
  using StringPtr = Ptr<std::string>;
  struct SchemaRef {
    void *itsSchema{}; // Break circular dependency Storage <-> Schema<Storage>
    bool operator==(const SchemaRef &theOther) const {
      return itsSchema == theOther.itsSchema;
    }
    bool operator!=(const SchemaRef &theOther) const {
      return itsSchema != theOther.itsSchema;
    }
  };
  struct SchemaPtr : public SchemaRef {
    using element_type = SchemaRef;
    static constexpr SchemaPtr pointer_to(const element_type &theRef) noexcept {
      return {{theRef.itsSchema}};
    }
    element_type operator*() const { return *this; }
    operator bool() const { return itsSchema; }
  };
  using JsonRef = Ref<cjson::DynamicDocument>;
  using JsonPtr = Ptr<cjson::DynamicDocument>;

  template <typename T> constexpr static auto pointer_to(const Ref<T> &theRef) {
    return &theRef.get();
  }
  static constexpr auto pointer_to(const SchemaRef &theRef) {
    return SchemaPtr{theRef};
  }
};

template <typename Standard_> struct DynamicSchemaContext {
  using Standard = Standard_;
  using Storage = DynamicStorage;

  template <typename T> using Buffer = std::vector<T>;
  template <typename Key, typename Value> using Map = Storage::Map<Key, Value>;
  using JsonStorage = cjson::DynamicDocument;

  using SchemaRef = typename Storage::SchemaRef;
  using SchemaPtr = typename Storage::SchemaPtr;
  using SchemaObject = typename Standard::template SchemaObject<Storage>;
  using JsonRef = typename Storage::JsonRef;
  using JsonAccessor = typename JsonStorage::EntityRef;
  using StringRef = typename Storage::StringRef;
  template <typename T>
  using BufferRef = typename Storage::template BufferRef<T>;
  template <typename Key, typename Value>
  using MapEntry = typename Storage::template MapEntry<Key, Value>;
  template <typename KeyT, typename ValT>
  using MapRef = typename Storage::template MapRef<KeyT, ValT>;

  template <typename ErrorHandling> struct Allocator {
    /// We use make_shared instead of make_unique since std::any does not allow
    /// move-only types. And implementing our own ain't that simple as well if
    /// we want small storage optimization.

    SchemaRef allocateSchema(const SchemaObject &theSchema,
                             DynamicSchemaContext &theContext) {
      auto aObject = std::make_shared<SchemaObject>(theSchema);
      SchemaObject *aObjectPointer = aObject.get();
      theContext.itsAllocations.emplace_back(std::move(aObject));
      return SchemaRef{aObjectPointer};
    }
    StringRef allocateString(DynamicSchemaContext &theContext,
                             const std::string_view &theStr) {
      auto aString =
          std::make_shared<std::string>(theStr.begin(), theStr.end());
      StringRef aResult{*aString};
      theContext.itsAllocations.emplace_back(std::move(aString));
      return aResult;
    }
    template <typename T>
    BufferRef<T> allocateBuffer(DynamicSchemaContext &theContext,
                                const size_t theSize, const type_tag<T>) {
      auto aBuffer = std::make_shared<Buffer<T>>();
      aBuffer->reserve(theSize);
      BufferRef<T> aBufferRef = *aBuffer;
      theContext.itsAllocations.emplace_back(std::move(aBuffer));
      return aBufferRef;
    }
    template <typename Key, typename Value>
    MapRef<Key, Value> allocateMap(DynamicSchemaContext &theContext,
                                   const size_t theSize,
                                   const type_tag<Key, Value>) {
      auto aMap = std::make_shared<Map<Key, Value>>();
      MapRef<Key, Value> aMapRef = *aMap;
      theContext.itsAllocations.emplace_back(std::move(aMap));
      return aMapRef;
    }
    template <typename JSON>
    JsonRef allocateJson(DynamicSchemaContext &theContext,
                         const JSON &theJson) {
      cjson::DocumentInfo aDocInfo = cjson::DocumentInfo::read(theJson);
      auto aJson = std::make_shared<JsonStorage>(aDocInfo);
      cjson::DocumentAllocator<JsonStorage, cjson::ErrorWillThrow> aAlloc;
      aJson->itsEntities[0] = aAlloc.allocateJson(*aJson, theJson);
      JsonRef aJsonRef = *aJson;
      theContext.itsAllocations.emplace_back(std::move(aJson));
      return aJsonRef;
    }
  };

  template <typename T, bool IsConst> class BufferAccessor;

  template <bool IsConst = true> struct AccessDecorator {
    using ContextTy = std::conditional_t<IsConst, const DynamicSchemaContext,
                                         DynamicSchemaContext>;

    explicit constexpr AccessDecorator(ContextTy &theContext)
        : itsContext(&theContext) {}

    template <typename T2>
    constexpr auto decorate(BufferRef<T2> theBuffer) const {
      return BufferAccessor<T2, IsConst>{*itsContext, theBuffer};
    }
    template <typename T1, typename T2>
    constexpr auto decorate(const std::pair<T1, T2> &thePair) const {
      return std::make_pair(decorate(thePair.first), decorate(thePair.second));
    }
    constexpr bool decorate(const bool theBool) const { return theBool; }
    constexpr Types decorate(const Types theType) const { return theType; }
    std::string_view decorate(StringRef theString) const {
      return theString.get();
    }
    auto decorate(JsonRef theJson) const { return theJson.get().getRoot(); }
    constexpr auto decorate(SchemaRef theSchema) const {
      return SchemaObjectAccessor<DynamicSchemaContext>{*itsContext, theSchema};
    }
    template <typename... Args>
    constexpr auto operator()(Args &&...theArgs) const {
      return decorate(std::forward<Args>(theArgs)...);
    }
    constexpr bool operator==(const AccessDecorator &) const { return true; }

  private:
    ContextTy *itsContext;
  };

  template <typename T, bool IsConst = true> class BufferAccessor {
  public:
    using ContextTy = std::conditional_t<IsConst, const DynamicSchemaContext,
                                         DynamicSchemaContext>;
    using Value = std::conditional_t<IsConst, const T, T>;
    constexpr BufferAccessor(ContextTy &theContext, BufferRef<T> theBuffer)
        : itsContext(&theContext), itsBuffer(theBuffer) {}

    constexpr size_t size() const { return itsBuffer.get().size(); }
    constexpr auto operator[](const ptrdiff_t theIdx) const {
      AccessDecorator<IsConst> aDecorator{*itsContext};
      return aDecorator.decorate(itsBuffer.get()[theIdx]);
    }

    constexpr bool operator==(const BufferAccessor &theOther) const {
      return itsContext == theOther.itsContext &&
             itsBuffer.get() == theOther.itsBuffer.get();
    }
    constexpr bool operator!=(const BufferAccessor &theOther) const {
      return !(*this == theOther);
    }

    using Iterator = SubscriptIterator<BufferAccessor>;
    constexpr Iterator begin() const { return {*this, 0}; }
    constexpr Iterator end() const {
      return {*this, static_cast<ptrdiff_t>(itsBuffer.get().size())};
    }

  private:
    ContextTy *itsContext;
    BufferRef<T> itsBuffer;
  };

  template <typename KeyT, typename ValT, bool IsConst = true>
  struct MapAccessor {
    using ContextTy = std::conditional_t<IsConst, const DynamicSchemaContext,
                                         DynamicSchemaContext>;

    using Iterator = DecoratingIterator<
        std::conditional_t<IsConst, typename Map<KeyT, ValT>::const_iterator,
                           typename Map<KeyT, ValT>::iterator>,
        AccessDecorator<IsConst>>;

    Iterator begin() const {
      return Iterator{itsMap.get().begin(),
                      AccessDecorator<IsConst>{*itsContext}};
    }
    Iterator end() const {
      return Iterator{itsMap.get().end(),
                      AccessDecorator<IsConst>{*itsContext}};
    }
    bool operator==(const MapAccessor &theOther) const {
      return itsContext == theOther.itsContext &&
             itsMap.get() == theOther.itsMap.get();
    }
    bool operator!=(const MapAccessor &theOther) const {
      return !(*this == theOther);
    }
    size_t size() const { return itsMap.get().size(); }

    constexpr MapAccessor(ContextTy &theContext, MapRef<KeyT, ValT> theMap)
        : itsContext(&theContext), itsMap(theMap) {}

    constexpr auto operator[](const std::string_view &theKey) const {
      AccessDecorator<IsConst> aDecorator;
      using LookupResult =
          std::optional<decltype(aDecorator(std::declval<const ValT &>()))>;
      auto aIter = itsMap.get().find(theKey);
      if (aIter != itsMap.get().end()) {
        return LookupResult{aDecorator(*aIter)};
      }
      return LookupResult{std::nullopt};
    }

  private:
    ContextTy *itsContext;
    MapRef<KeyT, ValT> itsMap;
  };

  template <typename T>
  constexpr void extendBuffer(BufferRef<T> theList, const T &theValue) {
    theList.get().push_back(theValue);
  }
  template <typename KeyT, typename ValT>
  constexpr void addMapEntry(MapRef<KeyT, ValT> theMap, const KeyT &theKey,
                             const ValT &theVal) {
    theMap.get().emplace(theKey, theVal);
  }

  constexpr const SchemaObject &getSchemaObject(const SchemaRef &theRef) const {
    return *static_cast<SchemaObject *>(theRef.itsSchema);
  }

  constexpr void setBoolSchemas(const SchemaRef &theTrue,
                                const SchemaRef &theFalse) {
    itsTrueSchema = SchemaPtr::pointer_to(theTrue);
    itsFalseSchema = SchemaPtr::pointer_to(theFalse);
  }
  constexpr bool isTrueSchema(const SchemaRef &theRef) const noexcept {
    return SchemaPtr::pointer_to(theRef) == itsTrueSchema;
  }
  constexpr bool isFalseSchema(const SchemaRef &theRef) const noexcept {
    return SchemaPtr::pointer_to(theRef) == itsFalseSchema;
  }

  std::string_view getString(StringRef theStr) const { return theStr.get(); }
  JsonAccessor getJson(JsonRef theJson) const {
    return theJson.get().getRoot();
  }

private:
  template <typename ErrorHandling> friend struct Allocator;
  std::vector<std::any> itsAllocations;
  SchemaPtr itsTrueSchema;
  SchemaPtr itsFalseSchema;
};
} // namespace json_schema
#endif // JSON_SCHEMA_DYNAMIC_SCHEMA_H
