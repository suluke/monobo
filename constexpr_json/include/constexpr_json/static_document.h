#ifndef CONSTEXPR_JSON_STATIC_DOCUMENT_H
#define CONSTEXPR_JSON_STATIC_DOCUMENT_H

#include "constexpr_json/document.h"

namespace cjson {
namespace impl {
template <ssize_t theNumNumbers, ssize_t theNumChars, ssize_t theNumStrings,
          ssize_t theNumArrays, ssize_t theNumArrayEntries,
          ssize_t theNumObjects, ssize_t theNumObjectProperties>
struct StaticDocumentStorage {
  template <typename T, size_t N> using Buffer = std::array<T, N>;

  static constexpr ssize_t MAX_NUMBERS() { return theNumNumbers; }
  static constexpr ssize_t MAX_CHARS() { return theNumChars; }
  static constexpr ssize_t MAX_ENTITIES() {
    return theNumArrayEntries + theNumObjectProperties + 1;
  }
  static constexpr ssize_t MAX_ARRAYS() { return theNumArrays; }
  static constexpr ssize_t MAX_OBJECTS() { return theNumObjects; }
  static constexpr ssize_t MAX_OBJECT_PROPS() { return theNumObjectProperties; }
  static constexpr ssize_t MAX_STRINGS() { return theNumStrings; }

  template <typename T, size_t N>
  static constexpr Buffer<T, N> createBuffer(size_t theSize) {
    return Buffer<T, N>{};
  }
};

template <typename StaticDocTy> struct StaticDocumentAccessor {
  constexpr StaticDocumentAccessor(const StaticDocTy &theDoc)
      : itsDoc{&theDoc} {}

  constexpr bool operator==(const StaticDocumentAccessor& theOther) const noexcept {
    return itsDoc == theOther.itsDoc;
  }

  constexpr double getNumber(intptr_t theIdx) const {
    return itsDoc->itsNumbers[theIdx];
  }
  constexpr std::string_view getString(intptr_t theIdx) const {
    const String &aStr = itsDoc->itsStrings[theIdx];
    return std::string_view{itsDoc->itsChars.data() + aStr.itsPosition,
                            aStr.itsSize};
  }
  constexpr const Entity *array_begin(intptr_t theIdx) const {
    return &itsDoc->itsEntities[itsDoc->itsArrays[theIdx].itsPosition];
  }
  constexpr const Entity *array_end(intptr_t theIdx) const {
    return array_begin(theIdx) + array_size(theIdx);
  }
  constexpr size_t array_size(intptr_t theIdx) const {
    return itsDoc->itsArrays[theIdx].itsNumElements;
  }
  constexpr size_t getNumProperties(intptr_t theObjIdx) const {
    return itsDoc->itsObjects[theObjIdx].itsNumProperties;
  }
  constexpr std::pair<std::string_view, typename StaticDocTy::EntityRef>
  getProperty(intptr_t theObjIdx, intptr_t thePropIdx) const {
    const Object &aObject = itsDoc->itsObjects[theObjIdx];
    std::string_view aKey = getString(
        itsDoc->itsObjectProps[aObject.itsKeysPos + thePropIdx].itsKeyPos);
    typename StaticDocTy::EntityRef aValue{
        *this, itsDoc->itsEntities[aObject.itsValuesPos + thePropIdx]};
    return std::make_pair(aKey, aValue);
  }

  // FIXME typesystem hack. Remove if this can be compiled with c++20's constexpr reference_wrapper
  constexpr const StaticDocumentAccessor *operator->() const { return this; }

private:
  const StaticDocTy* itsDoc;
};
} // namespace impl

template <ssize_t theNumNumbers, ssize_t theNumChars, ssize_t theNumStrings,
          ssize_t theNumArrays, ssize_t theNumArrayEntries,
          ssize_t theNumObjects, ssize_t theNumObjectProperties>
struct StaticDocument
    : public DocumentBase<impl::StaticDocumentStorage<
          theNumNumbers, theNumChars, theNumStrings, theNumArrays,
          theNumArrayEntries, theNumObjects, theNumObjectProperties>> {
  using BaseClass = DocumentBase<impl::StaticDocumentStorage<
      theNumNumbers, theNumChars, theNumStrings, theNumArrays,
      theNumArrayEntries, theNumObjects, theNumObjectProperties>>;
  using Self =
      StaticDocument<theNumNumbers, theNumChars, theNumStrings, theNumArrays,
                     theNumArrayEntries, theNumObjects, theNumObjectProperties>;
  using Storage =
      impl::StaticDocumentStorage<theNumNumbers, theNumChars, theNumStrings,
                                  theNumArrays, theNumArrayEntries,
                                  theNumObjects, theNumObjectProperties>;
  using EntityRef = impl::EntityRefImpl<impl::StaticDocumentAccessor<Self>>;

  constexpr StaticDocument(const DocumentInfo &theDocInfo)
      : DocumentBase<Storage>{theDocInfo} {}

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

  constexpr inline EntityRef getStaticRoot() const {
    return {*this, BaseClass::itsEntities[0]};
  }
};
} // namespace cjson
#endif // CONSTEXPR_JSON_STATIC_DOCUMENT_H
