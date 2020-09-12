#ifndef CONSTEXPR_JSON_DOCUMENT_BUILDER_H
#define CONSTEXPR_JSON_DOCUMENT_BUILDER_H

#include "impl/document_builder2.h"

#include <memory>

namespace cjson {
template <typename SourceEncodingTy, typename DestEncodingTy>
using DocumentBuilderImpl = DocumentBuilder2<SourceEncodingTy, DestEncodingTy>;

template <typename SourceEncodingTy, typename DestEncodingTy,
          template <typename SEncTy, typename DEncTy> class Impl =
              DocumentBuilderImpl>
struct DocumentBuilder : public Impl<SourceEncodingTy, DestEncodingTy> {
  using BaseClass = Impl<SourceEncodingTy, DestEncodingTy>;

  constexpr static DocumentInfo
  computeDocInfo(const std::string_view theJsonString) {
    return DocumentInfo::compute<SourceEncodingTy, DestEncodingTy>(
               theJsonString)
        .first;
  }

  using BaseClass::parseDocument;
};
} // namespace cjson
#endif // CONSTEXPR_JSON_DOCUMENT_BUILDER_H
