#ifndef CONSTEXPR_JSON_DOCUMENT_H
#define CONSTEXPR_JSON_DOCUMENT_H

#include <array>
#include <cstddef>
#include <string_view>
namespace cjson {

struct Entity {
  enum KIND { ARRAY, BOOLEAN, DOUBLE, NUL, OBJECT, STRING } itsKind;
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

struct DocumentBase {
  virtual Entity getRoot() const = 0;
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

  Entity getRoot() const override { return itsRoot; }
};

} // namespace cjson
#endif // CONSTEXPR_JSON_DOCUMENT_H