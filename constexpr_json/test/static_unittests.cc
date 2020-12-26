#include "constexpr_json/document_builder.h"
#include "constexpr_json/ext/error_is_detail.h"
#include "constexpr_json/impl/document_builder1.h"
#include "constexpr_json/static_document.h"

#include <cmath>

using namespace cjson;

#define CHECK_UTF8_DECODE(STR, EXPECTED)                                       \
  do {                                                                         \
    static_assert(Utf8::decodeFirst(STR).first == EXPECTED,                    \
                  "UTF8: Failed to decode '" STR "'");                         \
  } while (false)

template <intptr_t L> struct CompareCharSeqs {
  bool value = true;
  template <typename T1, typename T2>
  constexpr CompareCharSeqs(T1 theFirst, T2 theSecond) {
    if constexpr (L <= 0) {
      std::ignore = theFirst;
      std::ignore = theSecond;
      return;
    } else {
      constexpr intptr_t aIdx = L - 1;
      if (theFirst[aIdx] != theSecond[aIdx]) {
        value = false;
        return;
      }
      CompareCharSeqs<aIdx> aRemCmp(theFirst, theSecond);
      value = aRemCmp.value;
    }
  }
};

#define CHECK_UTF8_ENCODE(CODEPOINT, EXPECTED)                                 \
  do {                                                                         \
    constexpr std::string_view aExpStr{EXPECTED};                              \
    constexpr auto aResult = Utf8::encode(CODEPOINT);                          \
    constexpr intptr_t aExpLen = static_cast<intptr_t>(aExpStr.size());        \
    static_assert(aResult.second == aExpLen);                                  \
    constexpr intptr_t aMinLen =                                               \
        aResult.second < aExpLen ? aResult.second : aExpLen;                   \
    constexpr CompareCharSeqs<aMinLen> aCmp(aResult.first, aExpStr);           \
    static_assert(aCmp.value);                                                 \
  } while (false)

static void test_utf8() {
  // Test utf8 decoder
  CHECK_UTF8_DECODE("$", 0x24);
  CHECK_UTF8_DECODE("¢", 0xa2);
  CHECK_UTF8_DECODE("ह", 0x939);
  CHECK_UTF8_DECODE("€", 0x20ac);
  CHECK_UTF8_DECODE("𐍈", 0x10348);

  // Test utf8 encoder
  CHECK_UTF8_ENCODE(0x24, "$");
  CHECK_UTF8_ENCODE(0xa2, "¢");
  CHECK_UTF8_ENCODE(0x939, "ह");
  CHECK_UTF8_ENCODE(0x20ac, "€");
}

#undef CHECK_UTF8_DECODE
#undef CHECK_UTF8_ENCODE

#define CHECK_READ(FN, STR, EXPECTED)                                          \
  do {                                                                         \
    static_assert(parsing<Utf8>::FN(STR) == EXPECTED,                          \
                  #FN ": Failed to read '" STR "'");                           \
  } while (false)

#define CHECK_PARSE(FN, STR, EXPECTED_LEN, EXPECTED_ELM)                       \
  do {                                                                         \
    using namespace std::literals;                                             \
    constexpr auto aParsed = parsing<Utf8>::FN(STR##sv);                       \
    constexpr auto aElem = aParsed.first;                                      \
    constexpr intptr_t aElemLength = aParsed.second;                           \
    constexpr auto aExpectedElm = EXPECTED_ELM;                                \
    static_assert(EXPECTED_LEN == aElemLength);                                \
    static_assert(aExpectedElm == aElem);                                      \
  } while (false)

static void test_parseutils() {
  // Test parsing procedures
  //   readWhitespace
  CHECK_READ(readWhitespace, "", "");
  CHECK_READ(readWhitespace, "     ", "     ");
  CHECK_READ(readWhitespace, "     abc", "     ");
  CHECK_READ(readWhitespace, "\na", "\n");
  CHECK_READ(readWhitespace, "\ra", "\r");
  CHECK_READ(readWhitespace, "\ta", "\t");

  //   readDigits
  CHECK_READ(readDigits, "", "");
  CHECK_READ(readDigits, " 123abc", "");
  CHECK_READ(readDigits, "123abc", "123");

  //   readFraction
  CHECK_READ(readFraction, "", "");
  CHECK_READ(readFraction, ".", "");
  CHECK_READ(readFraction, ".1", ".1");

  //   readString
  CHECK_READ(readString, "", "");
  CHECK_READ(readString, "\"\"", "\"\"");
  CHECK_READ(readString, "\"\\n\"", "\"\\n\"");
  CHECK_READ(readString, "\"\uabcd\"", "\"\uabcd\"");

  //   readNumber
  CHECK_READ(readNumber, "1,", "1");
  CHECK_READ(readNumber, "1.", "1");
  CHECK_READ(readNumber, "1.23", "1.23");
  CHECK_READ(readNumber, "-", "");
  CHECK_READ(readNumber, "0123", "0");
  CHECK_READ(readNumber, "1.23e45", "1.23e45");
  CHECK_READ(readNumber, "1.23E45", "1.23E45");
  CHECK_READ(readNumber, "-1.23E45", "-1.23E45");

  //   parseInt
  CHECK_PARSE(parseInteger, "", -1, 0.);
  CHECK_PARSE(parseInteger, "0123", 1, 0.);
  CHECK_PARSE(parseInteger, "-0123", 2, -0.);
  static_assert(parsing<Utf8>::parseInteger("-0123").first == -0.);
  CHECK_PARSE(parseInteger, "1", 1, 1.);
  CHECK_PARSE(parseInteger, "-1", 2, -1.);
  CHECK_PARSE(parseInteger, "10", 2, 10.);
  CHECK_PARSE(parseInteger, "-10", 3, -10.);
  CHECK_PARSE(parseInteger, "-10abc", 3, -10.);

  //   parseExponent
  CHECK_PARSE(parseExponent, "e0", 2, 0);
  CHECK_PARSE(parseExponent, "E0", 2, 0);
  CHECK_PARSE(parseExponent, "E1", 2, 1);
  CHECK_PARSE(parseExponent, "E+1", 3, 1);
  CHECK_PARSE(parseExponent, "E-3", 3, -3);

  //   parseNumber
  CHECK_PARSE(parseNumber, "0", 1, 0.);
  CHECK_PARSE(parseNumber, "01", 1, 0.);
  CHECK_PARSE(parseNumber, "1", 1, 1.);
  CHECK_PARSE(parseNumber, "1234", 4, 1234.);
  CHECK_PARSE(parseNumber, "-1", 2, -1.);
  CHECK_PARSE(parseNumber, "-1.2", 4, -1.2);
  CHECK_PARSE(parseNumber, "-1.0e3", 6, -100.);
  CHECK_PARSE(parseNumber, "-1.0e-3", 7, -0.001);
  CHECK_PARSE(parseNumber, "-1.0e-03", 8, -0.001);
  CHECK_PARSE(parseNumber, "2]", 1, 2.);

  CHECK_PARSE(parseEscape, "\\u0001", 6, 1);
  CHECK_PARSE(parseEscape, "\\u000f", 6, 15);
  CHECK_PARSE(parseEscape, "\\u0010", 6, 16);
  CHECK_PARSE(parseEscape, "\\u00ff", 6, 255);
  CHECK_PARSE(parseEscape, "\\u0100", 6, 256);
  CHECK_PARSE(parseEscape, "\\u0fff", 6, 4095);
  CHECK_PARSE(parseEscape, "\\u1000", 6, 4096);

  //   readNull
  CHECK_READ(readNull, "abcdee", "");
  CHECK_READ(readNull, "null", "null");
  CHECK_READ(readNull, "nullabcd", "null");

  //   parseBool
  CHECK_PARSE(parseBool, "abcdee", -1, false);
  CHECK_PARSE(parseBool, "true", 4, true);
  CHECK_PARSE(parseBool, "false", 5, false);
  CHECK_PARSE(parseBool, "trueabc", 4, true);
  CHECK_PARSE(parseBool, "falseabc", 5, false);
}

template <typename ErrorHandling> static void test_docinfo() {
#define CHECK_COUNTS(NULLS, BOOLS, DOUBLES, CHARS, STRS, ARRAYS, ENTRIES,      \
                     OBJECTS, PROPS, LENGTH, JSON)                             \
  do {                                                                         \
    constexpr const auto aResultOrError =                                      \
        DocumentInfo::compute<Utf8, Utf8, ErrorHandling>(JSON);                \
    static_assert(!ErrorHandling::isError(aResultOrError));                    \
    constexpr auto aResult = ErrorHandling::unwrap(aResultOrError);            \
    constexpr const DocumentInfo aDI = aResult.first;                          \
    constexpr const DocumentInfo aExpected = {                                 \
        NULLS, BOOLS, DOUBLES, CHARS, STRS, ARRAYS, ENTRIES, OBJECTS, PROPS};  \
    static_assert(aResult.second == LENGTH);                                   \
    static_assert(aDI);                                                        \
    static_assert(aDI == aExpected);                                           \
    static_assert(aDI == aExpected);                                           \
  } while (false)

  // DocInfo computation
  //            N  B  D  C  S  A  a  O  o  L
  CHECK_COUNTS(1, 0, 0, 0, 0, 0, 0, 0, 0, 4, "null");
  CHECK_COUNTS(0, 1, 0, 0, 0, 0, 0, 0, 0, 4, "true");
  CHECK_COUNTS(0, 1, 0, 0, 0, 0, 0, 0, 0, 5, "false");
  CHECK_COUNTS(0, 0, 1, 0, 0, 0, 0, 0, 0, 6, "1.2e-3");
  CHECK_COUNTS(0, 0, 0, 3, 1, 0, 0, 0, 0, 5, "\"abc\"");
  CHECK_COUNTS(0, 0, 0, 2, 1, 0, 0, 0, 0, 4, "\"¢\"");
  CHECK_COUNTS(0, 0, 0, 1, 1, 0, 0, 0, 0, 4, "\"\\n\"");
  CHECK_COUNTS(0, 0, 0, 1, 1, 0, 0, 0, 0, 8, "\"\\u002a\"");
  CHECK_COUNTS(0, 0, 0, 0, 0, 0, 0, 1, 0, 2, "{}");
  CHECK_COUNTS(0, 0, 0, 3, 1, 1, 0, 1, 1, 11, "{\"abc\": []}");
  CHECK_COUNTS(0, 0, 0, 6, 2, 2, 0, 1, 2, 22, "{\"abc\": [], \"def\": []}");
  CHECK_COUNTS(1, 0, 0, 1, 1, 0, 0, 1, 1, 11, "{\"a\": null}");
  CHECK_COUNTS(0, 1, 1, 9, 3, 1, 0, 1, 3, 34,
               "{\"abc\": [], \"def\": 1, \"ghi\": true}");
  CHECK_COUNTS(0, 0, 0, 0, 0, 1, 0, 0, 0, 2, "[]");
  CHECK_COUNTS(0, 0, 0, 3, 1, 1, 1, 0, 0, 7, "[\"abc\"]");
  CHECK_COUNTS(0, 0, 0, 9, 3, 1, 3, 0, 0, 19, "[\"abc\",\"def\",\"ghi\"]");

#undef CHECK_COUNTS
}

template <typename Builder = DocumentBuilder<Utf8, Utf8>>
static void test_parsing() {
#define CHECK_DOCPARSE(JSON)                                                   \
  do {                                                                         \
    using ErrorHandling = typename Builder::error_handling;                    \
    constexpr std::string_view aJsonStr{JSON};                                 \
    constexpr const auto aDocInfoOrError = Builder::computeDocInfo(aJsonStr);  \
    static_assert(!ErrorHandling::isError(aDocInfoOrError));                   \
    constexpr const DocumentInfo aDocInfo =                                    \
        ErrorHandling::unwrap(aDocInfoOrError);                                \
    using DocTy = CJSON_STATIC_DOCTY(aDocInfo);                                \
    constexpr auto aDoc =                                                      \
        Builder::template parseDocument<DocTy>(aJsonStr, aDocInfo);            \
    static_assert(!ErrorHandling::isError(aDoc));                              \
    /*dump(Builder::unwrap(aDoc));*/                                           \
    /*std::cout << Builder::unwrap(aDoc) << "\n";*/                            \
  } while (false)
  CHECK_DOCPARSE("null");
  CHECK_DOCPARSE("true");
  CHECK_DOCPARSE("false");
  CHECK_DOCPARSE("123");
  CHECK_DOCPARSE("\"123\"");
  CHECK_DOCPARSE("[3,2,1, \"123\",[null,true,false]]");
  CHECK_DOCPARSE("  {\n    \"Test\": false,\n    \"Toast\":true,\n    "
                 "\"Tasty\":[null]\n}  ");
  CHECK_DOCPARSE("\"\\u01ff\"");
  {
    constexpr std::string_view aJsonStr{
        "[123,true,null,\"abc\",{\"def\":\"ghi\"}]"};
    constexpr auto aDocInfoOrError = Builder::computeDocInfo(aJsonStr);
    using ErrorHandling = typename Builder::error_handling;
    static_assert(!ErrorHandling::isError(aDocInfoOrError));
    constexpr DocumentInfo aDocInfo{ErrorHandling::unwrap(aDocInfoOrError)};
    using DocTy = CJSON_STATIC_DOCTY(aDocInfo);
    constexpr auto aDocOrError =
        Builder::template parseDocument<DocTy>(aJsonStr, aDocInfo);
    static_assert(!ErrorHandling::isError(aDocOrError));
    constexpr const DocTy aDoc = ErrorHandling::unwrap(aDocOrError);
    static_assert(aDoc.getRoot().toArray().size() == 5);

    static_assert(aDoc.getRoot().toArray()[1].toBool());
    static_assert(aDoc.getRoot().toArray()[3].toString() == "abc");
    // gcc-7 does not consider optional::operator-> to be constexpr...
    static_assert((*aDoc.getRoot().toArray()[4].toObject()["def"]).toString() ==
                  "ghi");
  }
#undef CHECK_DOCPARSE
}

int main() {
  test_utf8();
  test_parseutils();
  test_docinfo<ErrorWillReturnNone>();
  test_docinfo<ErrorWillReturnDetail<JsonErrorDetail>>();
  using ErrorHandling = ErrorWillReturnNone;
  test_parsing<DocumentBuilder<Utf8, Utf8, ErrorHandling, DocumentBuilder1>>();
  test_parsing<DocumentBuilder<Utf8, Utf8, ErrorHandling, DocumentBuilder2>>();
  test_parsing();
  return 0;
}
