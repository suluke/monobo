#ifndef CONSTEXPR_JSON_DOCUMENT_H
#define CONSTEXPR_JSON_DOCUMENT_H

#include "constexpr_json/document_info.h"
#include "constexpr_json/impl/document_access.h"

#include <array>
#include <functional>
#include <string_view>
namespace cjson {
struct DocumentInterface {
  using EntityRef = impl::EntityRefImpl<DocumentInterface>;

  virtual ~DocumentInterface() = default;

  /// Satisfy requirements for EntityRef
  /// @{
  virtual double getNumber(intptr_t theIdx) const = 0;
  virtual std::string_view getString(intptr_t theIdx) const = 0;
  virtual const Entity *array_begin(intptr_t theIdx) const = 0;
  virtual const Entity *array_end(intptr_t theIdx) const = 0;
  virtual size_t array_size(intptr_t theIdx) const = 0;
  virtual size_t getNumProperties(intptr_t theObjIdx) const = 0;
  virtual std::pair<std::string_view, const Entity *>
  getProperty(intptr_t theObjIdx, intptr_t thePropIdx) const = 0;
  virtual const Entity *
  getProperty(const intptr_t theObjIdx,
              const std::string_view theKey) const noexcept = 0;
  /// @}

  virtual EntityRef getRoot() const noexcept = 0;
  bool operator==(const DocumentInterface &theOther) const noexcept {
    return getRoot() == theOther.getRoot();
  }
};

template <typename Storage> struct DocumentBase {
  template <typename T, size_t N>
  using Buffer = typename Storage::template Buffer<T, N>;

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

  constexpr double getNumber(intptr_t theIdx) const {
    return itsNumbers[theIdx];
  }
  constexpr std::string_view getString(intptr_t theIdx) const {
    const String &aStr = itsStrings[theIdx];
    return std::string_view{itsChars.data() + aStr.itsPosition, aStr.itsSize};
  }
  constexpr const Entity *array_begin(intptr_t theIdx) const {
    return &itsEntities[itsArrays[theIdx].itsPosition];
  }
  constexpr const Entity *array_end(intptr_t theIdx) const {
    return array_begin(theIdx) + array_size(theIdx);
  }
  constexpr size_t array_size(intptr_t theIdx) const {
    return itsArrays[theIdx].itsNumElements;
  }
  constexpr size_t getNumProperties(intptr_t theObjIdx) const {
    return itsObjects[theObjIdx].itsNumProperties;
  }
  constexpr std::pair<std::string_view, const Entity *>
  getProperty(intptr_t theObjIdx, intptr_t thePropIdx) const {
    const Object &aObject = itsObjects[theObjIdx];
    std::string_view aKey =
        getString(itsObjectProps[aObject.itsKeysPos + thePropIdx].itsKeyPos);
    const Entity &aValue = itsEntities[aObject.itsValuesPos + thePropIdx];
    return std::make_pair(aKey, &aValue);
  }
  constexpr const Entity *
  getProperty(const intptr_t theObjIdx,
              const std::string_view theKey) const noexcept {
    // perform linear search
    const Object &aObject = itsObjects[theObjIdx];
    for (intptr_t aPropIdx = 0;
         static_cast<size_t>(aPropIdx) < aObject.itsNumProperties; ++aPropIdx) {
      const auto aProp = itsObjectProps[aObject.itsKeysPos + aPropIdx];
      if (getString(aProp.itsKeyPos) == theKey)
        return &itsEntities[aObject.itsValuesPos + aPropIdx];
    }
    return nullptr;
  }

  constexpr const Entity &getRootEntity() const noexcept {
    return itsEntities[0];
  }
};

template <typename Base>
struct DocumentInterfaceImpl : public DocumentInterface, Base {
  using EntityRef = DocumentInterface::EntityRef;

  template <typename... Args>
  DocumentInterfaceImpl(Args &&... theArgs)
      : Base(std::forward<Args>(theArgs)...) {}

  EntityRef getRoot() const noexcept override {
    return {*this, Base::getRootEntity()};
  }
  double getNumber(intptr_t theIdx) const override {
    return Base::getNumber(theIdx);
  }
  std::string_view getString(intptr_t theIdx) const override {
    return Base::getString(theIdx);
  }
  const Entity *array_begin(intptr_t theIdx) const override {
    return Base::array_begin(theIdx);
  }
  const Entity *array_end(intptr_t theIdx) const override {
    return Base::array_end(theIdx);
  }
  size_t array_size(intptr_t theIdx) const override {
    return Base::array_size(theIdx);
  }
  size_t getNumProperties(intptr_t theObjIdx) const override {
    return Base::getNumProperties(theObjIdx);
  }
  std::pair<std::string_view, const Entity *>
  getProperty(intptr_t theObjIdx, intptr_t thePropIdx) const override {
    return Base::getProperty(theObjIdx, thePropIdx);
  }
  const Entity *
  getProperty(const intptr_t theObjIdx,
              const std::string_view theKey) const noexcept override {
    return Base::getProperty(theObjIdx, theKey);
  }
};
} // namespace cjson
#endif // CONSTEXPR_JSON_DOCUMENT_H
