#ifndef CONSTEXPR_JSON_DOCUMENT_H
#define CONSTEXPR_JSON_DOCUMENT_H

#include "document_info.h"

#include <array>
#include <cstddef>
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
struct EntityRef {
public:
  EntityRef(const DocumentInterface &theDoc, const Entity &theEntity)
      : itsDoc(&theDoc), itsEntity(&theEntity) {}
  EntityRef() = default;
  EntityRef(const EntityRef &) = default;
  EntityRef(EntityRef &&) = default;
  EntityRef &operator=(const EntityRef &) = default;
  EntityRef &operator=(EntityRef &&) = default;

  Entity::KIND getType() const { return itsEntity->itsKind; }
  bool toBool() const { return itsEntity->itsPayload; }
  double toNumber() const;
  std::string_view toString() const;
  std::vector<EntityRef> toArray() const;
  std::map<std::string_view, EntityRef> toObject() const;

private:
  const DocumentInterface *itsDoc;
  const Entity *itsEntity;
};

struct DocumentInterface {
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

template <ssize_t theNumNumbers, ssize_t theNumChars, ssize_t theNumStrings,
          ssize_t theNumArrays, ssize_t theNumArrayEntries,
          ssize_t theNumObjects, ssize_t theNumObjectProperties>
struct Document : public DocumentInterface {
  constexpr Document(const DocumentInfo &theDocInfo)
      : itsNumbers{createBuffer<double, theNumNumbers>(
            theDocInfo.itsNumNumbers)},
        itsChars{createBuffer<char, theNumChars>(theDocInfo.itsNumChars)},
        itsEntities{createBuffer<Entity, MAX_ENTITIES()>(
            theDocInfo.itsNumObjectProperties + theDocInfo.itsNumArrayEntries +
            1)},
        itsArrays{createBuffer<Array, theNumArrays>(theDocInfo.itsNumArrays)},
        itsObjects{
            createBuffer<Object, theNumObjects>(theDocInfo.itsNumObjects)},
        itsObjectProps{createBuffer<Property, theNumObjectProperties>(
            theDocInfo.itsNumObjectProperties)},
        itsStrings{
            createBuffer<String, theNumStrings>(theDocInfo.itsNumStrings)} {}

  static_assert(theNumNumbers >= 0,
                "Negative NumNumbers for document is illegal");
  static_assert(theNumChars >= 0, "Negative NumChars for document is illegal");
  static_assert(theNumStrings >= 0,
                "Negative NumStrings for document is illegal");
  static_assert(theNumArrays >= 0,
                "Negative NumArrays for document is illegal");
  static_assert(theNumArrayEntries >= 0,
                "Negative NumArrayEntries for document is illegal");
  static_assert(theNumObjects >= 0,
                "Negative NumObjects for document is illegal");
  static_assert(theNumObjectProperties >= 0,
                "Negative NumObjectProperties for document is illegal");

  static constexpr ssize_t MAX_ENTITIES() {
    return theNumArrayEntries + theNumObjectProperties + 1;
  }

  template <typename T, size_t N> using Buffer = std::array<T, N>;

  template <typename T, size_t N>
  static constexpr Buffer<T, N> createBuffer(size_t theSize) {
    return Buffer<T, N>{};
  }

  Buffer<double, theNumNumbers> itsNumbers;
  Buffer<char, theNumChars> itsChars;
  Buffer<Entity, MAX_ENTITIES()> itsEntities;
  Buffer<Array, theNumArrays> itsArrays;
  Buffer<Object, theNumObjects> itsObjects;
  Buffer<Property, theNumObjectProperties> itsObjectProps;
  Buffer<String, theNumStrings> itsStrings;

  EntityRef getRoot() const override { return {*this, itsEntities[0]}; }
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
    EntityRef aValue{*this, itsEntities[aObject.itsValuesPos + thePropIdx]};
    return std::make_pair(aKey, aValue);
  }
};

double EntityRef::toNumber() const {
  return itsDoc->getNumber(itsEntity->itsPayload);
}
std::string_view EntityRef::toString() const {
  return itsDoc->getString(itsEntity->itsPayload);
}
std::vector<EntityRef> EntityRef::toArray() const {
  intptr_t aIdx = itsEntity->itsPayload;
  std::vector<EntityRef> aArray;
  aArray.reserve(itsDoc->array_size(aIdx));
  for (const Entity *aIter = itsDoc->array_begin(aIdx),
                    *aEnd = itsDoc->array_end(aIdx);
       aIter != aEnd; ++aIter) {
    aArray.emplace_back(*itsDoc, *aIter);
  }
  return aArray;
}
std::map<std::string_view, EntityRef> EntityRef::toObject() const {
  intptr_t aObjectIdx = itsEntity->itsPayload;
  std::map<std::string_view, EntityRef> aObject;
  for (size_t aPropIdx = 0u; aPropIdx < itsDoc->getNumProperties(aObjectIdx);
       ++aPropIdx)
    aObject.emplace(itsDoc->getProperty(aObjectIdx, aPropIdx));
  std::ignore = aObject;
  return aObject;
}

} // namespace cjson
#endif // CONSTEXPR_JSON_DOCUMENT_H
