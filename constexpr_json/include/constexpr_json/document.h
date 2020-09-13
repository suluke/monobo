#ifndef CONSTEXPR_JSON_DOCUMENT_H
#define CONSTEXPR_JSON_DOCUMENT_H

#include "document_info.h"

#include <array>
#include <cstddef>
#include <functional>
#include <map>
#include <string_view>
#include <vector>
namespace cjson {
using ssize_t = std::make_signed_t<size_t>;

struct Entity {
  enum KIND { NUL = 0, ARRAY, BOOL, NUMBER, OBJECT, STRING } itsKind = NUL;
  intptr_t itsPayload = 0;
};
struct Array {
  intptr_t itsPosition; // index into itsEntities
  size_t itsNumElements;
};
struct Object {
  intptr_t itsKeysPos;   // index into itsObjectProps
  intptr_t itsValuesPos; // index into itsEntities
  size_t itsNumProperties;
};
struct Property {
  intptr_t itsKeyPos; // index into itsStrings
};
struct String {
  intptr_t itsPosition; // index into itsChars
  size_t itsSize;
};

struct DocumentInterface;
namespace impl {
template <typename DocumentRefType> struct EntityRefImpl {
public:
  constexpr EntityRefImpl(const DocumentRefType theDoc,
                          const Entity &theEntity)
      : itsDoc(theDoc), itsEntity(&theEntity) {}
  constexpr EntityRefImpl() = delete;
  constexpr EntityRefImpl(const EntityRefImpl &) = default;
  constexpr EntityRefImpl(EntityRefImpl &&) = default;
  constexpr EntityRefImpl &operator=(const EntityRefImpl &) = default;
  constexpr EntityRefImpl &operator=(EntityRefImpl &&) = default;

  constexpr Entity::KIND getType() const { return itsEntity->itsKind; }
  constexpr bool toBool() const { return itsEntity->itsPayload; }
  constexpr double toNumber() const;
  constexpr std::string_view toString() const;
  constexpr std::vector<EntityRefImpl> toArray() const;
  constexpr std::map<std::string_view, EntityRefImpl> toObject() const;

private:
  DocumentRefType itsDoc = {};
  const Entity *itsEntity = nullptr;
};
} // namespace impl

struct DocumentInterface {
  using EntityRef = impl::EntityRefImpl<const DocumentInterface *>;

  virtual EntityRef getRoot() const = 0;
  virtual double getNumber(intptr_t theIdx) const = 0;
  virtual std::string_view getString(intptr_t theIdx) const = 0;
  virtual const Entity *array_begin(intptr_t theIdx) const = 0;
  virtual const Entity *array_end(intptr_t theIdx) const = 0;
  virtual size_t array_size(intptr_t theIdx) const = 0;
  virtual size_t getNumProperties(intptr_t theObjIdx) const = 0;
  virtual std::pair<std::string_view, EntityRef>
  getProperty(intptr_t theObjIdx, intptr_t thePropIdx) const = 0;
};

template <typename Storage> struct DocumentBase : public DocumentInterface {
  template <typename T, size_t N> using Buffer = typename Storage::Buffer<T, N>;
  using EntityRef = DocumentInterface::EntityRef;

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

  EntityRef getRoot() const override { return {this, itsEntities[0]}; }
  double getNumber(intptr_t theIdx) const override {
    return itsNumbers[theIdx];
  }
  std::string_view getString(intptr_t theIdx) const override {
    const String &aStr = itsStrings[theIdx];
    return std::string_view{itsChars.data() + aStr.itsPosition, aStr.itsSize};
  }
  const Entity *array_begin(intptr_t theIdx) const override {
    return &itsEntities[itsArrays[theIdx].itsPosition];
  }
  const Entity *array_end(intptr_t theIdx) const override {
    return array_begin(theIdx) + array_size(theIdx);
  }
  size_t array_size(intptr_t theIdx) const override {
    return itsArrays[theIdx].itsNumElements;
  }
  size_t getNumProperties(intptr_t theObjIdx) const override {
    return itsObjects[theObjIdx].itsNumProperties;
  }
  std::pair<std::string_view, EntityRef>
  getProperty(intptr_t theObjIdx, intptr_t thePropIdx) const override {
    const Object &aObject = itsObjects[theObjIdx];
    std::string_view aKey =
        getString(itsObjectProps[aObject.itsKeysPos + thePropIdx].itsKeyPos);
    EntityRef aValue{this, itsEntities[aObject.itsValuesPos + thePropIdx]};
    return std::make_pair(aKey, aValue);
  }
};

namespace impl {
template <typename DocumentType>
constexpr double EntityRefImpl<DocumentType>::toNumber() const {
  return itsDoc->getNumber(itsEntity->itsPayload);
}
template <typename DocumentType>
constexpr std::string_view EntityRefImpl<DocumentType>::toString() const {
  return itsDoc->getString(itsEntity->itsPayload);
}
template <typename DocumentType>
constexpr std::vector<EntityRefImpl<DocumentType>>
EntityRefImpl<DocumentType>::toArray() const {
  intptr_t aIdx = itsEntity->itsPayload;
  std::vector<EntityRefImpl<DocumentType>> aArray;
  aArray.reserve(itsDoc->array_size(aIdx));
  for (const Entity *aIter = itsDoc->array_begin(aIdx),
                    *aEnd = itsDoc->array_end(aIdx);
       aIter != aEnd; ++aIter) {
    aArray.emplace_back(itsDoc, *aIter);
  }
  return aArray;
}
template <typename DocumentType>
constexpr std::map<std::string_view, EntityRefImpl<DocumentType>>
EntityRefImpl<DocumentType>::toObject() const {
  intptr_t aObjectIdx = itsEntity->itsPayload;
  std::map<std::string_view, EntityRefImpl<DocumentType>> aObject;
  for (size_t aPropIdx = 0u; aPropIdx < itsDoc->getNumProperties(aObjectIdx);
       ++aPropIdx)
    aObject.emplace(itsDoc->getProperty(aObjectIdx, aPropIdx));
  std::ignore = aObject;
  return aObject;
}
} // namespace impl
} // namespace cjson
#endif // CONSTEXPR_JSON_DOCUMENT_H
