#ifndef CONSTEXPR_JSON_DOCUMENT_BUILDER_H
#define CONSTEXPR_JSON_DOCUMENT_BUILDER_H

#include "constexpr_json/impl/document_builder2.h"
#include "constexpr_json/ext/utf-8.h"
#include "constexpr_json/ext/error_is_nullopt.h"

#include <optional>
#include <memory>

namespace cjson {
/// Select the default builder implementation
///
/// Currently DocumentBuilder2
template <typename SourceEncodingTy, typename DestEncodingTy,
          typename ErrorHandlingTy = ErrorWillReturnNone>
using DocumentBuilderImpl =
    DocumentBuilder2<SourceEncodingTy, DestEncodingTy, ErrorHandlingTy>;

/// Core API of cjson
///
/// Basically only a facade in front of the builder implementation.
template <typename SourceEncodingTy = Utf8, typename DestEncodingTy = Utf8,
          typename ErrorHandlingTy = ErrorWillReturnNone,
          template <typename SEncTy, typename DEncTy, typename ErrHandTy>
          class Impl = DocumentBuilderImpl>
struct DocumentBuilder
    : public Impl<SourceEncodingTy, DestEncodingTy, ErrorHandlingTy> {
  using BaseClass = Impl<SourceEncodingTy, DestEncodingTy, ErrorHandlingTy>;

  /// Compute a DocumentInfo object for the given JSON string
  constexpr static typename ErrorHandlingTy::template ErrorOr<DocumentInfo>
  computeDocInfo(const std::string_view theJsonString) {
    const auto aDocInfoOrError =
        DocumentInfo::compute<SourceEncodingTy, DestEncodingTy,
                              ErrorHandlingTy>(theJsonString);
    if (ErrorHandlingTy::isError(aDocInfoOrError))
      return ErrorHandlingTy::template convertError<DocumentInfo>(aDocInfoOrError);
    return ErrorHandlingTy::unwrap(aDocInfoOrError).first;
  }

  using BaseClass::parseDocument;

  using src_encoding = SourceEncodingTy;
  using dest_encoding = DestEncodingTy;
  using error_handling = ErrorHandlingTy;
};
} // namespace cjson
#endif // CONSTEXPR_JSON_DOCUMENT_BUILDER_H
