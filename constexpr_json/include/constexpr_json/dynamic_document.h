#ifndef CONSTEXPR_JSON_DYNAMIC_DOCUMENT_H
#define CONSTEXPR_JSON_DYNAMIC_DOCUMENT_H

#include "constexpr_json/document.h"

#include <limits>

namespace cjson {
namespace impl {
struct DynamicDocumentStorage {
  template <typename T, size_t N> using Buffer = std::vector<T>;

  static constexpr intptr_t MAX_NUMBERS() {
    return std::numeric_limits<intptr_t>::max();
  }
  static constexpr intptr_t MAX_CHARS() {
    return std::numeric_limits<intptr_t>::max();
  }
  static constexpr intptr_t MAX_ENTITIES() {
    return std::numeric_limits<intptr_t>::max();
  }
  static constexpr intptr_t MAX_ARRAYS() {
    return std::numeric_limits<intptr_t>::max();
  }
  static constexpr intptr_t MAX_OBJECTS() {
    return std::numeric_limits<intptr_t>::max();
  }
  static constexpr intptr_t MAX_OBJECT_PROPS() {
    return std::numeric_limits<intptr_t>::max();
  }
  static constexpr intptr_t MAX_STRINGS() {
    return std::numeric_limits<intptr_t>::max();
  }

  template <typename T, size_t N>
  static Buffer<T, N> createBuffer(size_t theSize) {
    Buffer<T, N> aBuf;
    aBuf.resize(theSize);
    return aBuf;
  }
};
} // namespace impl

struct DynamicDocument : public DocumentBase<impl::DynamicDocumentStorage> {
  using Storage = impl::DynamicDocumentStorage;

  DynamicDocument(const DocumentInfo &theDocInfo)
      : DocumentBase<Storage>{theDocInfo} {}
};
} // namespace cjson
#endif // CONSTEXPR_JSON_DYNAMIC_DOCUMENT_H
