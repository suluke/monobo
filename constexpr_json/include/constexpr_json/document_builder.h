#ifndef CONSTEXPR_JSON_DOCUMENT_BUILDER_H
#define CONSTEXPR_JSON_DOCUMENT_BUILDER_H

#include "constexpr_json/impl/document_builder2.h"
#include "constexpr_json/impl/error_handling.h"
#include "constexpr_json/utils/unicode.h"

#include <memory>

namespace cjson {
template <typename SourceEncodingTy, typename DestEncodingTy,
          typename ErrorHandlingTy = ErrorWillReturnNone>
using DocumentBuilderImpl =
    DocumentBuilder2<SourceEncodingTy, DestEncodingTy, ErrorHandlingTy>;

template <typename SourceEncodingTy = Utf8, typename DestEncodingTy = Utf8,
          typename ErrorHandlingTy = ErrorWillReturnNone,
          template <typename SEncTy, typename DEncTy, typename ErrHandTy>
          class Impl = DocumentBuilderImpl>
struct DocumentBuilder
    : public Impl<SourceEncodingTy, DestEncodingTy, ErrorHandlingTy> {
  using BaseClass = Impl<SourceEncodingTy, DestEncodingTy, ErrorHandlingTy>;

  constexpr static typename ErrorHandlingTy::ErrorOr<DocumentInfo>
  computeDocInfo(const std::string_view theJsonString) {
    return DocumentInfo::compute<SourceEncodingTy, DestEncodingTy,
                                 ErrorHandlingTy>(theJsonString)
        .first;
  }

  using BaseClass::parseDocument;

  using src_encoding = SourceEncodingTy;
  using dest_encoding = DestEncodingTy;
  using error_handling = ErrorHandlingTy;

  template <typename T>
  static constexpr bool
  isError(const typename error_handling::ErrorOr<T> &theValue) noexcept {
    return error_handling::isError(theValue);
  }

  template <typename T>
  static constexpr const T &
  unwrap(const typename error_handling::ErrorOr<T> &theValue) noexcept {
    return error_handling::unwrap(theValue);
  }
};
} // namespace cjson
#endif // CONSTEXPR_JSON_DOCUMENT_BUILDER_H
