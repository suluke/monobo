#include "constexpr_json/document_parser.h"
#include "constexpr_json/ext/printing.h"
#include "constexpr_json/static_document.h"

using namespace cjson;

struct CharCountingStream {
  using Self = CharCountingStream;
  size_t itsSize{0};

  constexpr Self operator<<(const char theChar) const noexcept {
    return {itsSize + 1};
  }
  constexpr Self operator<<(const std::string_view theString) const noexcept {
    return {itsSize + theString.size()};
  }
  constexpr size_t size() const noexcept { return itsSize; }
};

#define TEST_IDEMPOTENT(theJson)                                               \
  do {                                                                         \
    constexpr std::string_view aJson{theJson};                                 \
    /* 1: Parse theJson */                                                     \
    using Parser = DocumentParser<>;                                           \
    constexpr const DocumentInfo aDocInfo = *Parser::computeDocInfo(aJson);    \
    using DocTy = CJSON_STATIC_DOCTY(aDocInfo);                                \
    constexpr const DocTy aDoc =                                               \
        *Parser::template parseDocument<DocTy>(aJson, aDocInfo);               \
    /* 2: Compute required buffer size for printing */                         \
    using CounterTy = Printer<Utf8, Utf8, CharCountingStream>;                 \
    constexpr size_t aPrintedSize =                                            \
        CounterTy::print(CharCountingStream{}, aDoc.getRoot()).size();         \
    /* 3: Print */                                                             \
    using StreamTy = StaticStream<aPrintedSize>;                               \
    using PrinterTy = Printer<Utf8, Utf8, StreamTy>;                           \
    constexpr StreamTy aStream = PrinterTy::print(StreamTy{}, aDoc.getRoot()); \
    /* 4: Parse the printed result again */                                    \
    constexpr const DocumentInfo aDocInfoNew =                                 \
        *Parser::computeDocInfo(aStream.str());                                \
    using DocTyNew = CJSON_STATIC_DOCTY(aDocInfoNew);                          \
    constexpr const DocTyNew aDocNew =                                         \
        *Parser::template parseDocument<DocTyNew>(aStream.str(), aDocInfo);    \
    /* 5: Assert the re-parsed result is equal to the original document */     \
    static_assert(aDoc.getRoot() == aDocNew.getRoot());                        \
  } while (false)

//#undef TEST_IDEMPOTENT // uncomment this for runtime debugging
#ifndef TEST_IDEMPOTENT
#define TEST_IDEMPOTENT(theJson)                                               \
  do {                                                                         \
    constexpr std::string_view aJson{theJson};                                 \
    /* 1: Parse theJson */                                                     \
    using Parser = DocumentParser<>;                                           \
    constexpr const DocumentInfo aDocInfo = *Parser::computeDocInfo(aJson);    \
    using DocTy = CJSON_STATIC_DOCTY(aDocInfo);                                \
    constexpr const DocTy aDoc =                                               \
        *Parser::template parseDocument<DocTy>(aJson, aDocInfo);               \
    /* 2: Print */                                                             \
    using PrinterTy = Printer<Utf8, Utf8>;                                     \
    PrinterTy::print(std::cout, aDoc.getRoot());                               \
    std::cout << "\n";                                                         \
  } while (false)
#endif

int main() {
  TEST_IDEMPOTENT("100");
  TEST_IDEMPOTENT("0.99999999999999999");
  TEST_IDEMPOTENT("0.1");
  TEST_IDEMPOTENT("0.3");
  TEST_IDEMPOTENT("0.6");
  TEST_IDEMPOTENT(
      "{\"abc\":false,\"def\":\"test\",\"ghi\":[123.456],\"jkl\":null}");
#define USE_JSON_STRING(theJson)                                               \
  constexpr std::string_view aJsonSchema{theJson};
#include "json_schema.h"
  TEST_IDEMPOTENT(aJsonSchema);
  return 0;
}
