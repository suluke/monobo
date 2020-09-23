#include "constexpr_json/document_builder.h"
#include "constexpr_json/ext/printing.h"
#include "constexpr_json/static_document.h"

using namespace cjson;

template <size_t BufferSize> struct StaticStream {
  std::array<char, BufferSize> itsBuffer;
  size_t itsSize{0};

  constexpr StaticStream() noexcept : itsBuffer{} {}

  constexpr StaticStream operator<<(const char theChar) const noexcept {
    StaticStream aCopy{*this};
    aCopy.itsBuffer[itsSize] = theChar;
    ++aCopy.itsSize;
    return aCopy;
  }
  constexpr StaticStream
  operator<<(const std::string_view theString) const noexcept {
    StaticStream aCopy{*this};
    for (const char aChar : theString)
      aCopy = (aCopy << aChar);
    return aCopy;
  }
  constexpr StaticStream operator<<(const double theNumber) const noexcept {
    return (*this) << '0'; // TODO
  }
  constexpr std::string_view str() const noexcept {
    return {itsBuffer.begin(), itsSize};
  }
};

#define TEST_IDEMPOTENT(theJson)                                               \
  do {                                                                         \
    constexpr std::string_view aJson{theJson};                                 \
    using Builder = DocumentBuilder<>;                                         \
    constexpr const DocumentInfo aDocInfo = *Builder::computeDocInfo(aJson);   \
    using DocTy = CJSON_STATIC_DOCTY(aDocInfo);                                \
    constexpr const DocTy aDoc =                                               \
        *Builder::template parseDocument<DocTy>(aJson, aDocInfo);              \
    using StreamTy = StaticStream<aJson.size()>;                               \
    using PrinterTy = Printer<Utf8, Utf8, StreamTy>;                           \
    constexpr StreamTy aStream =                                               \
        PrinterTy::print(StreamTy{}, aDoc.getStaticRoot());                    \
    static_assert(aDoc.getStaticRoot().getType() == Entity::OBJECT);           \
    static_assert(aJson.size() == aStream.str().size());                       \
    static_assert(aJson == aStream.str());                                     \
  } while (false)

int main() {
  TEST_IDEMPOTENT("{\"abc\":false,\"def\":\"test\",\"ghi\":[0],\"jkl\":null}");
  return 0;
}
