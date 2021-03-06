#ifndef CONSTEXPR_JSON_DYNAMIC_DOCUMENT_H
#define CONSTEXPR_JSON_DYNAMIC_DOCUMENT_H

#include "constexpr_json/document.h"
#include "constexpr_json/document_parser.h"

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

struct DynamicDocument
    : public DocumentInterfaceImpl<DocumentBase<impl::DynamicDocumentStorage>> {
  using Base =
      DocumentInterfaceImpl<DocumentBase<impl::DynamicDocumentStorage>>;
  using Storage = impl::DynamicDocumentStorage;

  DynamicDocument(const DocumentInfo &theDocInfo) : Base{theDocInfo} {}

  template <typename Parser>
  using ParseResult = typename Parser::error_handling::template ErrorOr<
      std::unique_ptr<cjson::DynamicDocument>>;

  /// Parses a JSON string into a DynamicDocument using the specified Parser
  /// type
  template <typename Parser = DocumentParser<>>
  static ParseResult<Parser>
  parseJson(const std::string_view theJson,
            const typename Parser::src_encoding theSrcEnc = {},
            const typename Parser::dest_encoding theDestEnc = {}) {
    using ErrorHandling = typename Parser::error_handling;
    using ResultTy = std::unique_ptr<DynamicDocument>;
    const auto aDocInfoOrError =
        DocumentInfo::compute<typename Parser::src_encoding,
                              typename Parser::dest_encoding,
                              typename Parser::error_handling>(theJson, theSrcEnc, theDestEnc);

    if (ErrorHandling::isError(aDocInfoOrError))
      return ErrorHandling::template convertError<ResultTy>(aDocInfoOrError);
    const auto aDocInfoAndLen = ErrorHandling::unwrap(aDocInfoOrError);
    const DocumentInfo aDocInfo = aDocInfoAndLen.first;
    const intptr_t aDocSize = aDocInfoAndLen.second;
    assert(aDocInfo);
    using P = parsing<typename Parser::src_encoding>;
    const P p{theSrcEnc};
    // Only trailing whitespace is allowed behind parsing end
    if (!p.removeLeadingWhitespace(theJson.substr(aDocSize)).empty())
      return ErrorHandling::template makeError<ResultTy>(
          ErrorCode::TRAILING_CONTENT, aDocSize);
    const auto aDocOrError =
        Parser::template parseDocument<DynamicDocument>(theJson, aDocInfo, theSrcEnc, theDestEnc);
    if (ErrorHandling::isError(aDocOrError))
      return ErrorHandling::template convertError<ResultTy>(aDocOrError);
    auto aResult = std::make_unique<DynamicDocument>(aDocInfo);
    *aResult = std::move(ErrorHandling::unwrap(aDocOrError));
    return {std::move(aResult)};
  }
};
} // namespace cjson
#endif // CONSTEXPR_JSON_DYNAMIC_DOCUMENT_H
