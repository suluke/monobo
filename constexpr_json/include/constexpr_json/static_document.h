#ifndef CONSTEXPR_JSON_STATIC_DOCUMENT_H
#define CONSTEXPR_JSON_STATIC_DOCUMENT_H

#include "document.h"

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
} // namespace impl

template <ssize_t theNumNumbers, ssize_t theNumChars, ssize_t theNumStrings,
          ssize_t theNumArrays, ssize_t theNumArrayEntries,
          ssize_t theNumObjects, ssize_t theNumObjectProperties>
struct StaticDocument
    : public DocumentBase<impl::StaticDocumentStorage<
          theNumNumbers, theNumChars, theNumStrings, theNumArrays,
          theNumArrayEntries, theNumObjects, theNumObjectProperties>> {
  using Storage =
      impl::StaticDocumentStorage<theNumNumbers, theNumChars, theNumStrings,
                                  theNumArrays, theNumArrayEntries,
                                  theNumObjects, theNumObjectProperties>;

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
};
} // namespace cjson
#endif // CONSTEXPR_JSON_STATIC_DOCUMENT_H
