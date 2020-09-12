#ifndef CONSTEXPR_JSON_DOCUMENT_BUILDER_H
#define CONSTEXPR_JSON_DOCUMENT_BUILDER_H

#include "document_builder2.h"

#include <memory>

namespace cjson {
template <typename SourceEncodingTy, typename DestEncodingTy>
using DocumentBuilderImpl = DocumentBuilder2<SourceEncodingTy, DestEncodingTy>;

template <typename SourceEncodingTy, typename DestEncodingTy>
struct DocumentBuilder : public DocumentBuilderImpl<SourceEncodingTy, DestEncodingTy> {
  using BaseClass = DocumentBuilderImpl<SourceEncodingTy, DestEncodingTy>;

  constexpr static DocumentInfo
  computeDocInfo(const std::string_view theJsonString) {
    return DocumentInfo::compute<SourceEncodingTy, DestEncodingTy>(theJsonString).first;
  }

  using BaseClass::parseDocument;
};
} // namespace cjson
#endif // CONSTEXPR_JSON_DOCUMENT_BUILDER_H
