#ifndef CONSTEXPR_JSON_DOCUMENT_H
#define CONSTEXPR_JSON_DOCUMENT_H

#include "constexpr_json/document_info.h"
#include "constexpr_json/impl/document_access.h"

#include <array>
#include <functional>
#include <string_view>
namespace cjson {
struct DocumentInterface {
  using EntityRef = impl::EntityRefImpl<const DocumentInterface *>;

  virtual ~DocumentInterface() = default;

  virtual EntityRef getRoot() const noexcept = 0;
  virtual double getNumber(intptr_t theIdx) const = 0;
  virtual std::string_view getString(intptr_t theIdx) const = 0;
  virtual const Entity *array_begin(intptr_t theIdx) const = 0;
  virtual const Entity *array_end(intptr_t theIdx) const = 0;
  virtual size_t array_size(intptr_t theIdx) const = 0;
  virtual size_t getNumProperties(intptr_t theObjIdx) const = 0;
  virtual std::pair<std::string_view, EntityRef>
  getProperty(intptr_t theObjIdx, intptr_t thePropIdx) const = 0;

  bool operator==(const DocumentInterface &theOther) const noexcept {
    return getRoot() == theOther.getRoot();
  }
};

template <typename Storage> struct DocumentBase {
  template <typename T, size_t N> using Buffer = typename Storage::template Buffer<T, N>;

  Buffer<double, Storage::MAX_NUMBERS()> itsNumbers;
  Buffer<char, Storage::MAX_CHARS()> itsChars;
  Buffer<Entity, Storage::MAX_ENTITIES()> itsEntities;
  Buffer<Array, Storage::MAX_ARRAYS()> itsArrays;
  Buffer<Object, Storage::MAX_OBJECTS()> itsObjects;
  Buffer<Property, Storage::MAX_OBJECT_PROPS()> itsObjectProps;
  Buffer<String, Storage::MAX_STRINGS()> itsStrings;

  constexpr DocumentBase(const DocumentInfo &theDocInfo)
      : itsNumbers{Storage::template createBuffer<double,
                                                  Storage::MAX_NUMBERS()>(
            theDocInfo.itsNumNumbers)},
        itsChars{Storage::template createBuffer<char, Storage::MAX_CHARS()>(
            theDocInfo.itsNumChars)},
        itsEntities{
            Storage::template createBuffer<Entity, Storage::MAX_ENTITIES()>(
                theDocInfo.itsNumObjectProperties +
                theDocInfo.itsNumArrayEntries + 1)},
        itsArrays{Storage::template createBuffer<Array, Storage::MAX_ARRAYS()>(
            theDocInfo.itsNumArrays)},
        itsObjects{
            Storage::template createBuffer<Object, Storage::MAX_OBJECTS()>(
                theDocInfo.itsNumObjects)},
        itsObjectProps{
            Storage::template createBuffer<Property,
                                           Storage::MAX_OBJECT_PROPS()>(
                theDocInfo.itsNumObjectProperties)},
        itsStrings{
            Storage::template createBuffer<String, Storage::MAX_STRINGS()>(
                theDocInfo.itsNumStrings)} {}
};

template<typename Base>
struct DocumentInterfaceImpl : public DocumentInterface, Base {
  using EntityRef = DocumentInterface::EntityRef;

  template<typename... Args>
  DocumentInterfaceImpl(Args&&... theArgs) : Base(std::forward<Args>(theArgs)...) {}

  EntityRef getRoot() const noexcept override { return {this, Base::itsEntities[0]}; }
  double getNumber(intptr_t theIdx) const override {
    return Base::itsNumbers[theIdx];
  }
  std::string_view getString(intptr_t theIdx) const override {
    const String &aStr = Base::itsStrings[theIdx];
    return std::string_view{Base::itsChars.data() + aStr.itsPosition, aStr.itsSize};
  }
  const Entity *array_begin(intptr_t theIdx) const override {
    return &Base::itsEntities[Base::itsArrays[theIdx].itsPosition];
  }
  const Entity *array_end(intptr_t theIdx) const override {
    return array_begin(theIdx) + array_size(theIdx);
  }
  size_t array_size(intptr_t theIdx) const override {
    return Base::itsArrays[theIdx].itsNumElements;
  }
  size_t getNumProperties(intptr_t theObjIdx) const override {
    return Base::itsObjects[theObjIdx].itsNumProperties;
  }
  std::pair<std::string_view, EntityRef>
  getProperty(intptr_t theObjIdx, intptr_t thePropIdx) const override {
    const Object &aObject = Base::itsObjects[theObjIdx];
    std::string_view aKey =
        getString(Base::itsObjectProps[aObject.itsKeysPos + thePropIdx].itsKeyPos);
    EntityRef aValue{this, Base::itsEntities[aObject.itsValuesPos + thePropIdx]};
    return std::make_pair(aKey, aValue);
  }
};
} // namespace cjson
#endif // CONSTEXPR_JSON_DOCUMENT_H
