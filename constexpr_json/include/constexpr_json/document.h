#ifndef CONSTEXPR_JSON_DOCUMENT_H
#define CONSTEXPR_JSON_DOCUMENT_H

#include <array>
#include <cstddef>
#include <string_view>
#include <vector>
namespace cjson {

struct Entity {
  enum KIND { NUL = 0, ARRAY, BOOL, NUMBER, OBJECT, STRING } itsKind = NUL;
  std::intptr_t itsPayload = 0;
};
struct Array {
  intptr_t itsPosition;
  size_t itsNumElements;
};
struct Object {
  intptr_t itsPosition;
  size_t itsNumProperties;
};
struct String {
  intptr_t itsPosition;
  size_t itsSize;
};

struct DocumentBase;
struct EntityRef {
public:
  EntityRef(const DocumentBase &theDoc, const Entity &theEntity)
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

private:
  const DocumentBase *itsDoc;
  const Entity *itsEntity;
};

struct DocumentBase {
  virtual EntityRef getRoot() const = 0;
  virtual double getNumber(intptr_t theIdx) const = 0;
  virtual std::string_view getString(intptr_t theIdx) const = 0;
  virtual const Entity *array_begin(intptr_t theIdx) const = 0;
  virtual const Entity *array_end(intptr_t theIdx) const = 0;
  virtual size_t array_size(intptr_t theIdx) const = 0;
};

template <ssize_t theNumNumbers, ssize_t theNumChars, ssize_t theNumStrings,
          ssize_t theNumArrays, ssize_t theNumArrayEntries,
          ssize_t theNumObjects, ssize_t theNumObjectProperties>
struct Document : public DocumentBase {
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

  static constexpr const ssize_t itsNumNumbers = theNumNumbers;
  static constexpr const ssize_t itsNumChars = theNumChars;
  static constexpr const ssize_t itsNumStrings = theNumStrings;
  static constexpr const ssize_t itsNumArrays = theNumArrays;
  static constexpr const ssize_t itsNumArrayEntries = theNumArrayEntries;
  static constexpr const ssize_t itsNumObjects = theNumObjects;
  static constexpr const ssize_t itsNumObjectProperties =
      theNumObjectProperties;
  static constexpr const ssize_t itsNumEntities =
      theNumArrayEntries + theNumObjectProperties + 1;

  std::array<double, theNumNumbers> itsNumbers = {};
  std::array<char, theNumChars> itsChars = {};
  std::array<Entity, itsNumEntities> itsEntities = {};
  std::array<Array, theNumArrays> itsArrays = {};
  std::array<Object, theNumObjects> itsObjects = {};
  std::array<String, theNumStrings> itsStrings = {};

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

} // namespace cjson
#endif // CONSTEXPR_JSON_DOCUMENT_H
