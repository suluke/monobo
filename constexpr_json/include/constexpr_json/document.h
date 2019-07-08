#ifndef CONSTEXPR_JSON_DOCUMENT_H
#define CONSTEXPR_JSON_DOCUMENT_H

#include <array>
#include <cstddef>
#include <string_view>
namespace cjson {

struct Entity {
  enum KIND { NUL = 0, ARRAY, BOOLEAN, DOUBLE, OBJECT, STRING } itsKind;
  std::intptr_t itsPayload;
};
struct Array {
  intptr_t itsPosition;
  size_t itsNumProperties;
};
struct Object {
  intptr_t itsPosition;
  size_t itsNumProperties;
};
struct String {
  intptr_t itsPosition;
  size_t itsNumProperties;
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
  double toDouble() const;

private:
  const DocumentBase *itsDoc;
  const Entity *itsEntity;
};

struct DocumentBase {
  virtual EntityRef getRoot() const = 0;
  virtual double getDouble(intptr_t theIdx) const = 0;
};

template <size_t NumDoubles, size_t NumChars, size_t NumStrings,
          size_t NumArrays, size_t NumArrayEntries, size_t NumObjects,
          size_t NumObjectProperties>
struct Document : public DocumentBase {
  static_assert(NumDoubles >= 0, "Negative NumDoubles for document is illegal");
  static_assert(NumChars >= 0, "Negative NumChars for document is illegal");
  static_assert(NumStrings >= 0, "Negative NumStrings for document is illegal");
  static_assert(NumArrays >= 0, "Negative NumArrays for document is illegal");
  static_assert(NumArrayEntries >= 0,
                "Negative NumArrayEntries for document is illegal");
  static_assert(NumObjects >= 0, "Negative NumObjects for document is illegal");
  static_assert(NumObjectProperties >= 0,
                "Negative NumObjectProperties for document is illegal");

  std::array<double, NumDoubles> itsDoubles;
  std::array<char, NumChars> itsChars;
  std::array<Entity, NumArrayEntries + NumObjectProperties> itsEntities;
  std::array<Array, NumArrays> itsArrays;
  std::array<Object, NumObjects> itsObjects;
  std::array<String, NumStrings> itsStrings;
  Entity itsRoot;

  EntityRef getRoot() const override { return {*this, itsRoot}; }
  double getDouble(intptr_t theIdx) const { return itsDoubles[theIdx]; }
};

double EntityRef::toDouble() const {
  return itsDoc->getDouble(itsEntity->itsPayload);
}

} // namespace cjson
#endif // CONSTEXPR_JSON_DOCUMENT_H