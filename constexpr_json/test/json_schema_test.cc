
#include "constexpr_json/document_builder.h"
#include "constexpr_json/dynamic_document.h"
#include "constexpr_json/impl/document_builder1.h"
#include "constexpr_json/impl/document_builder2.h"
#include "constexpr_json/static_document.h"

#include <cassert>
#include <cmath>
#include <iostream>
#include <variant>

using namespace cjson;

static std::ostream &operator<<(std::ostream &theStream,
                                const DocumentInfo &theDocInfo) {
  return theStream << "Nulls: " << theDocInfo.itsNumNulls << "\n"
                   << "Booleans: " << theDocInfo.itsNumBools << "\n"
                   << "Numbers: " << theDocInfo.itsNumNumbers << "\n"
                   << "Characters: " << theDocInfo.itsNumChars << "\n"
                   << "Strings: " << theDocInfo.itsNumStrings << "\n"
                   << "Arrays: " << theDocInfo.itsNumArrays << "\n"
                   << "Array Entries: " << theDocInfo.itsNumArrayEntries << "\n"
                   << "Objects: " << theDocInfo.itsNumObjects << "\n"
                   << "Object properties: " << theDocInfo.itsNumObjectProperties
                   << "\n";
}

template<typename EntityRef>
static std::ostream &print(std::ostream &theStream,
                                const EntityRef &theEntity) {
  switch (theEntity.getType()) {
  case Entity::BOOL:
    return theStream << (theEntity.toBool() ? "true" : "false");
  case Entity::NUMBER:
    return theStream << theEntity.toNumber();
  case Entity::NUL:
    return theStream << "null";
  case Entity::ARRAY: {
    theStream << "[";
    const auto aArray = theEntity.toArray();
    for (auto aIter = aArray.begin(), aEnd = aArray.end(); aIter != aEnd;
         ++aIter) {
      const auto &aEntity = *aIter;
      print(theStream, aEntity);
      auto aNext = aIter;
      ++aNext;
      if (aNext != aEnd)
        theStream << ", ";
    }
    theStream << "]";
    return theStream;
  }
  case Entity::OBJECT: {
    theStream << "{";
    const auto aObject = theEntity.toObject();
    for (auto aIter = aObject.begin(), aEnd = aObject.end(); aIter != aEnd;
         ++aIter) {
      const auto &[aKey, aEntity] = *aIter;
      print(theStream << "\"" << aKey << "\": ", aEntity);
      auto aNext = aIter;
      ++aNext;
      if (aNext != aEnd)
        theStream << ", ";
    }
    theStream << "}";
    return theStream;
  }
  case Entity::STRING:
    return theStream << "\"" << theEntity.toString() << "\"";
  }
  return theStream;
}

static std::ostream &operator<<(std::ostream &theStream,
                                const DocumentInterface::EntityRef &theEntity) {
  return print(theStream, theEntity);
}

static std::ostream &operator<<(std::ostream &theStream,
                                const DocumentInterface &theDoc) {
  return theStream << theDoc.getRoot();
}

static std::ostream &operator<<(std::ostream &theStream,
                                const Entity::KIND &theKind) {
  switch (theKind) {
  case Entity::NUL:
    return theStream << "N";
  case Entity::BOOL:
    return theStream << "B";
  case Entity::NUMBER:
    return theStream << "D";
  case Entity::STRING:
    return theStream << "S";
  case Entity::ARRAY:
    return theStream << "A";
  case Entity::OBJECT:
    return theStream << "O";
  }
  return theStream;
}

template <typename DocTy> static void dump(const DocTy &theDoc) {
  std::cout << "======================================\n"
            << "#Numbers: " << theDoc.itsNumbers.size() << "\nNumbers: \n";
  for (const double &aNum : theDoc.itsNumbers)
    std::cout << aNum << "\n";
  std::cout << "#Chars: " << theDoc.itsChars.size() << "\nChars: \n";
  for (const char &aChar : theDoc.itsChars)
    std::cout << aChar << "\n";
  std::cout << "#Strings: " << theDoc.itsStrings.size() << "\n";
  for (const String &aString : theDoc.itsStrings)
    std::cout << "[" << aString.itsPosition << ": " << aString.itsSize << "]"
              << "\n";
  std::cout << "#Arrays: " << theDoc.itsArrays.size() << "\n";
  for (const Array &aArray : theDoc.itsArrays)
    std::cout << "[" << aArray.itsPosition << ": " << aArray.itsNumElements
              << "]"
              << "\n";
  std::cout << "#Objects: " << theDoc.itsObjects.size() << "\n";
  for (const Object &aObject : theDoc.itsObjects)
    std::cout << "{" << aObject.itsKeysPos << "/" << aObject.itsValuesPos
              << ": " << aObject.itsNumProperties << "}"
              << "\n";
  std::cout << "#Entities: " << theDoc.itsEntities.size() << "\n";
  for (const Entity &aEntity : theDoc.itsEntities)
    std::cout << aEntity.itsKind << ":" << aEntity.itsPayload << "\n";
  std::cout << "======================================\n";
}

#define CHECK_UTF8_DECODE(STR, EXPECTED)                                       \
  do {                                                                         \
    static_assert(Utf8::decodeFirst(STR).first == EXPECTED,                    \
                  "UTF8: Failed to decode '" STR "'");                         \
  } while (false)

template <ssize_t L> struct CompareCharSeqs {
  bool value = true;
  template <typename T1, typename T2>
  constexpr CompareCharSeqs(T1 theFirst, T2 theSecond) {
    if constexpr (L <= 0) {
      std::ignore = theFirst;
      std::ignore = theSecond;
      return;
    } else {
      constexpr ssize_t aIdx = L - 1;
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
    constexpr ssize_t aExpLen = static_cast<ssize_t>(aExpStr.size());          \
    static_assert(aResult.second == aExpLen);                                  \
    constexpr ssize_t aMinLen =                                                \
        aResult.second < aExpLen ? aResult.second : aExpLen;                   \
    constexpr CompareCharSeqs<aMinLen> aCmp(aResult.first, aExpStr);           \
    static_assert(aCmp.value);                                                 \
  } while (false)

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
    constexpr ssize_t aElemLength = aParsed.second;                            \
    constexpr auto aExpectedElm = EXPECTED_ELM;                                \
    static_assert(EXPECTED_LEN == aElemLength);                                \
    static_assert(aExpectedElm == aElem);                                      \
  } while (false)

#define CHECK_COUNTS(JSON, NULLS, BOOLS, DOUBLES, CHARS, STRS, ARRAYS,         \
                     ENTRIES, OBJECTS, PROPS)                                  \
  do {                                                                         \
    constexpr const DocumentInfo aDI = Builder::computeDocInfo(JSON);          \
    constexpr const DocumentInfo aExpected = {                                 \
        NULLS, BOOLS, DOUBLES, CHARS, STRS, ARRAYS, ENTRIES, OBJECTS, PROPS};  \
    std::cout.setstate(std::ios_base::badbit); /* Disables cout */             \
    std::cout << aDI << "vs\n" << aExpected << "\n";                           \
    std::cout.clear(); /* Re-enable cout */                                    \
    static_assert(aDI == aExpected);                                           \
  } while (false)

#define CHECK_COUNTS2(NULLS, BOOLS, DOUBLES, CHARS, STRS, ARRAYS, ENTRIES,     \
                      OBJECTS, PROPS, LENGTH, JSON)                            \
  do {                                                                         \
    constexpr const auto aResult = DocumentInfo::compute<Utf8, Utf8>(JSON);    \
    constexpr const DocumentInfo aDI = aResult.first;                          \
    constexpr const DocumentInfo aExpected = {                                 \
        NULLS, BOOLS, DOUBLES, CHARS, STRS, ARRAYS, ENTRIES, OBJECTS, PROPS};  \
    static_assert(aResult.second == LENGTH);                                   \
    static_assert(aDI != DocumentInfo::error());                               \
    static_assert(aDI.assertSame(aExpected));                                  \
    std::cout.setstate(std::ios_base::badbit); /* Disables cout */             \
    std::cout << aDI << "vs\n" << aExpected << "\n";                           \
    std::cout.clear(); /* Re-enable cout */                                    \
    static_assert(aDI == aExpected);                                           \
  } while (false)

template <typename Builder = DocumentBuilder<Utf8, Utf8>>
static void static_tests() {
  // Test utf8 decoder
  CHECK_UTF8_DECODE("$", 0x24);
  CHECK_UTF8_DECODE("¢", 0xa2);
  CHECK_UTF8_DECODE("ह", 0x939);
  CHECK_UTF8_DECODE("€", 0x20ac);

  // Test utf8 encoder
  CHECK_UTF8_ENCODE(0x24, "$");
  CHECK_UTF8_ENCODE(0xa2, "¢");
  CHECK_UTF8_ENCODE(0x939, "ह");
  CHECK_UTF8_ENCODE(0x20ac, "€");

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
  static_assert(std::signbit(parsing<Utf8>::parseInteger("-0123").first));
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

  // DocInfo computation
  // null,bool,double,char,string,array,entries,object,props
  CHECK_COUNTS("null", 1, 0, 0, 0, 0, 0, 0, 0, 0);
  CHECK_COUNTS("true", 0, 1, 0, 0, 0, 0, 0, 0, 0);
  CHECK_COUNTS("false", 0, 1, 0, 0, 0, 0, 0, 0, 0);
  CHECK_COUNTS("1.2e-3", 0, 0, 1, 0, 0, 0, 0, 0, 0);
  CHECK_COUNTS("\"abc\"", 0, 0, 0, 3, 1, 0, 0, 0, 0);
  CHECK_COUNTS("\"¢\"", 0, 0, 0, 2, 1, 0, 0, 0, 0);
  CHECK_COUNTS("\"\\n\"", 0, 0, 0, 1, 1, 0, 0, 0, 0);
  CHECK_COUNTS("\"\\u002a\"", 0, 0, 0, 1, 1, 0, 0, 0, 0);
  CHECK_COUNTS("{}", 0, 0, 0, 0, 0, 0, 0, 1, 0);
  CHECK_COUNTS("{\"abc\": []}", 0, 0, 0, 3, 1, 1, 0, 1, 1);
  CHECK_COUNTS("{\"abc\": [], \"def\": 1, \"ghi\": true}", 0, 1, 1, 9, 3, 1, 0,
               1, 3);
  CHECK_COUNTS("[]", 0, 0, 0, 0, 0, 1, 0, 0, 0);
  CHECK_COUNTS("[\"abc\"]", 0, 0, 0, 3, 1, 1, 1, 0, 0);
  CHECK_COUNTS("[\"abc\",\"def\",\"ghi\"]", 0, 0, 0, 9, 3, 1, 3, 0, 0);
  //            N  B  D  C  S  A  a  O  o  L
  CHECK_COUNTS2(1, 0, 0, 0, 0, 0, 0, 0, 0, 4, "null");
  CHECK_COUNTS2(0, 1, 0, 0, 0, 0, 0, 0, 0, 4, "true");
  CHECK_COUNTS2(0, 1, 0, 0, 0, 0, 0, 0, 0, 5, "false");
  CHECK_COUNTS2(0, 0, 1, 0, 0, 0, 0, 0, 0, 6, "1.2e-3");
  CHECK_COUNTS2(0, 0, 0, 3, 1, 0, 0, 0, 0, 5, "\"abc\"");
  CHECK_COUNTS2(0, 0, 0, 2, 1, 0, 0, 0, 0, 4, "\"¢\"");
  CHECK_COUNTS2(0, 0, 0, 1, 1, 0, 0, 0, 0, 4, "\"\\n\"");
  CHECK_COUNTS2(0, 0, 0, 1, 1, 0, 0, 0, 0, 8, "\"\\u002a\"");
  CHECK_COUNTS2(0, 0, 0, 0, 0, 0, 0, 1, 0, 2, "{}");
  CHECK_COUNTS2(0, 0, 0, 3, 1, 1, 0, 1, 1, 11, "{\"abc\": []}");
  CHECK_COUNTS2(0, 0, 0, 6, 2, 2, 0, 1, 2, 22, "{\"abc\": [], \"def\": []}");
  CHECK_COUNTS2(1, 0, 0, 1, 1, 0, 0, 1, 1, 11, "{\"a\": null}");
  CHECK_COUNTS2(0, 1, 1, 9, 3, 1, 0, 1, 3, 34,
                "{\"abc\": [], \"def\": 1, \"ghi\": true}");
  CHECK_COUNTS2(0, 0, 0, 0, 0, 1, 0, 0, 0, 2, "[]");
  CHECK_COUNTS2(0, 0, 0, 3, 1, 1, 1, 0, 0, 7, "[\"abc\"]");
  CHECK_COUNTS2(0, 0, 0, 9, 3, 1, 3, 0, 0, 19, "[\"abc\",\"def\",\"ghi\"]");

#define CHECK_DOCPARSE(JSON)                                                   \
  do {                                                                         \
    constexpr std::string_view aJsonStr{JSON};                                 \
    constexpr DocumentInfo aDocInfo = Builder::computeDocInfo(aJsonStr);       \
    using DocTy =                                                              \
        StaticDocument<aDocInfo.itsNumNumbers, aDocInfo.itsNumChars,           \
                       aDocInfo.itsNumStrings, aDocInfo.itsNumArrays,          \
                       aDocInfo.itsNumArrayEntries, aDocInfo.itsNumObjects,    \
                       aDocInfo.itsNumObjectProperties>;                       \
    constexpr std::optional<DocTy> aDoc =                                      \
        Builder::template parseDocument<DocTy>(aJsonStr, aDocInfo);            \
    static_assert(aDoc);                                                       \
    /*dump(*aDoc);*/                                                           \
    /*std::cout << *aDoc << "\n";*/                                            \
  } while (false)
  CHECK_DOCPARSE("null");
  CHECK_DOCPARSE("true");
  CHECK_DOCPARSE("false");
  CHECK_DOCPARSE("123");
  CHECK_DOCPARSE("\"123\"");
  CHECK_DOCPARSE("[3,2,1, \"123\",[null,true,false]]");
  CHECK_DOCPARSE("  {\n    \"Test\": false,\n    \"Toast\":true,\n    "
                 "\"Tasty\":[null]\n}  ");

  {
    constexpr std::string_view aJsonStr{"[123,true,null,\"abc\"]"};
    constexpr DocumentInfo aDocInfo = Builder::computeDocInfo(aJsonStr);
    using DocTy =
        StaticDocument<aDocInfo.itsNumNumbers, aDocInfo.itsNumChars,
                       aDocInfo.itsNumStrings, aDocInfo.itsNumArrays,
                       aDocInfo.itsNumArrayEntries, aDocInfo.itsNumObjects,
                       aDocInfo.itsNumObjectProperties>;
    constexpr auto aDoc = Builder::template parseDocument<DocTy>(aJsonStr, aDocInfo);
    static_assert(aDoc->getStaticRoot().toArray().size() == 4);

    // FIXME the following code is only here for temporary testing/demo'ing of toArray and operator==(EntityRef)
    for (typename DocTy::EntityRef aEntity : aDoc->getStaticRoot().toArray()) {
      print(std::cout, aEntity) << "\n";
    }

    const auto aDynamicDoc =
        Builder::template parseDocument<DynamicDocument>(aJsonStr, aDocInfo);
    assert(aDynamicDoc == aDoc);
  }
}

int main() {
  static_tests<DocumentBuilder<Utf8, Utf8, DocumentBuilder1>>();
  static_tests<DocumentBuilder<Utf8, Utf8, DocumentBuilder2>>();
  static_tests();
#define USE_JSON_STRING(theJson) constexpr std::string_view aJsonSV{theJson};
#include "json_schema.h"
  constexpr DocumentInfo aDocInfo =
      DocumentInfo::compute<Utf8, Utf8>(aJsonSV).first;
  using DocTy =
      StaticDocument<aDocInfo.itsNumNumbers, aDocInfo.itsNumChars,
                     aDocInfo.itsNumStrings, aDocInfo.itsNumArrays,
                     aDocInfo.itsNumArrayEntries, aDocInfo.itsNumObjects,
                     aDocInfo.itsNumObjectProperties>;
  { // Test DocumentBuilder1
    using Builder = DocumentBuilder1<Utf8, Utf8>;
    constexpr auto aDoc = Builder::parseDocument<DocTy>(aJsonSV, aDocInfo);
    static_assert(aDoc);
    std::cout << "\n" << *aDoc << "\n";
  }
  { // Test DocumentBuilder2
    using Builder = DocumentBuilder2<Utf8, Utf8>;
    constexpr auto aDoc = Builder::parseDocument<DocTy>(aJsonSV, aDocInfo);
    static_assert(aDoc);
    std::cout << "\n" << *aDoc << "\n";
  }
  { // Test default DocumentBuilder
    using Builder = DocumentBuilder<Utf8, Utf8>;
    constexpr auto aDoc = Builder::parseDocument<DocTy>(aJsonSV, aDocInfo);
    static_assert(aDoc);
    std::cout << "\n" << *aDoc << "\n";
  }
  { // Test default DocumentBuilder with DynamicDocument (parsing at runtime)
    using Builder = DocumentBuilder<Utf8, Utf8>;
    const auto aDoc =
        Builder::parseDocument<DynamicDocument>(aJsonSV, aDocInfo);
    assert(aDoc);
    std::cout << "\n" << *aDoc << "\n";
  }
}
