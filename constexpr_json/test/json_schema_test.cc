#include "constexpr_json/document_builder.h"
#include <cassert>
#include <cmath>
#include <iostream>
#include <variant>

using namespace cjson;

static std::ostream &
operator<<(std::ostream &theStream,
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

static std::ostream &operator<<(std::ostream &theStream,
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
      theStream << aEntity;
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
      theStream << "\"" << aKey << "\": " << aEntity;
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
                                const DocumentBase &theDoc) {
  return theStream << theDoc.getRoot();
}

static std::ostream &operator<<(std::ostream &theStream,
                                const Entity::KIND &theKind) {
  switch (theKind) {
  case Entity::NUL:
    return theStream << "0";
  case Entity::BOOL:
    return theStream << "B";
  case Entity::NUMBER:
    return theStream << "N";
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
            << "#Numbers: " << DocTy::itsNumNumbers << "\nNumbers: \n";
  for (const double &aNum : theDoc.itsNumbers)
    std::cout << aNum << "\n";
  std::cout << "#Chars: " << DocTy::itsNumChars << "\nChars: \n";
  for (const char &aChar : theDoc.itsChars)
    std::cout << aChar << "\n";
  std::cout << "#Strings: " << DocTy::itsNumStrings << "\n";
  for (const String &aString : theDoc.itsStrings)
    std::cout << "[" << aString.itsPosition << ": " << aString.itsSize << "]"
              << "\n";
  std::cout << "#Arrays: " << DocTy::itsNumArrays << "\n";
  for (const Array &aArray : theDoc.itsArrays)
    std::cout << "[" << aArray.itsPosition << ": " << aArray.itsNumElements
              << "]"
              << "\n";
  std::cout << "#Entities: " << DocTy::itsNumEntities << "\n";
  for (const Entity &aEntity : theDoc.itsEntities)
    std::cout << aEntity.itsKind << ":" << aEntity.itsPayload << "\n";
  std::cout << "======================================\n";
}

struct JsonElement {
  Entity::KIND itsType = Entity::NUL;
  bool itsBoolVal = false;
  double itsNumberVal = 0.;
  std::string_view itsStringVal;
  size_t itsArrayLen = 0;
  size_t itsNumProperties = 0;
  constexpr void setBool(bool theBool) {
    itsType = Entity::BOOL;
    itsBoolVal = theBool;
  }
  constexpr void setNumber(double theNum) {
    itsType = Entity::NUMBER;
    itsNumberVal = theNum;
  }
  constexpr void setString(const std::string_view theStr) {
    itsType = Entity::STRING;
    itsStringVal = theStr;
  }
  constexpr void setNull() { itsType = Entity::NUL; }
  constexpr void setArray() {
    itsArrayLen = 0;
    itsType = Entity::ARRAY;
  }
  constexpr void addArrayEntry(const JsonElement &) { ++itsArrayLen; }
  constexpr void setObject() {
    itsNumProperties = 0;
    itsType = Entity::OBJECT;
  }
  constexpr void addObjectProperty(const std::string_view,
                                   const JsonElement &) {
    ++itsNumProperties;
  }
  constexpr bool operator==(const JsonElement &theOther) const {
    if (theOther.itsType != itsType)
      return false;
    switch (itsType) {
    case Entity::NUL:
      return true;
    case Entity::BOOL:
      return theOther.itsBoolVal == itsBoolVal;
    case Entity::NUMBER:
      return theOther.itsNumberVal == itsNumberVal;
    case Entity::STRING:
      return theOther.itsStringVal == itsStringVal;
    case Entity::ARRAY:
      return theOther.itsArrayLen == itsArrayLen;
    case Entity::OBJECT:
      return theOther.itsNumProperties == itsNumProperties;
    default:
      return false;
    }
  }
  constexpr static JsonElement null() {
    JsonElement aElm;
    aElm.setNull();
    return aElm;
  }
  constexpr static JsonElement boolean(bool theBool) {
    JsonElement aElm;
    aElm.setBool(theBool);
    return aElm;
  }
  constexpr static JsonElement number(double theNumber) {
    JsonElement aElm;
    aElm.setNumber(theNumber);
    return aElm;
  }
  constexpr static JsonElement string(std::string_view theString) {
    JsonElement aElm;
    aElm.setString(theString);
    return aElm;
  }
  constexpr static JsonElement array(size_t theSize) {
    JsonElement aElm;
    aElm.setArray();
    aElm.itsArrayLen = theSize;
    return aElm;
  }
  constexpr static JsonElement object(size_t theNumProps) {
    JsonElement aElm;
    aElm.setObject();
    aElm.itsNumProperties = theNumProps;
    return aElm;
  }
};

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
    using Builder = DocumentBuilder<Utf8, Utf8>;                               \
    constexpr const DocumentInfo aDI = Builder::computeDocInfo(JSON); \
    constexpr const DocumentInfo aExpected = {                        \
        NULLS, BOOLS, DOUBLES, CHARS, STRS, ARRAYS, ENTRIES, OBJECTS, PROPS};  \
    std::cout.setstate(std::ios_base::badbit); /* Disables cout */             \
    std::cout << aDI << "vs\n" << aExpected << "\n";                           \
    std::cout.clear(); /* Re-enable cout */                                    \
    static_assert(aDI == aExpected);                                           \
  } while (false)

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

  //   parseNull
  CHECK_READ(parseNull, "abcdee", -1);
  CHECK_READ(parseNull, "null", 4);
  CHECK_READ(parseNull, "nullabcd", 4);

  //   parseBool
  CHECK_PARSE(parseBool, "abcdee", -1, false);
  CHECK_PARSE(parseBool, "true", 4, true);
  CHECK_PARSE(parseBool, "false", 5, false);
  CHECK_PARSE(parseBool, "trueabc", 4, true);
  CHECK_PARSE(parseBool, "falseabc", 5, false);

  // Parsing elements
  //   parseArray
  CHECK_PARSE(parseArray<JsonElement>, "[\"a\", 2]  ", 8,
              JsonElement::array(2));
  CHECK_PARSE(parseArray<JsonElement>, "[\"a\", 2  ", -1, JsonElement::null());
  //   parseObject
  CHECK_PARSE(parseObject<JsonElement>, "{\"a\": 2}  ", 8,
              JsonElement::object(1));
  CHECK_PARSE(parseObject<JsonElement>, "{\"a\": 2, \"b\": \"c\"}  ", 18,
              JsonElement::object(2));
  CHECK_PARSE(parseObject<JsonElement>, "{\"a\": 2  ", -1, JsonElement::null());
  CHECK_PARSE(parseObject<JsonElement>, "{\"a\": 2,}", -1, JsonElement::null());
  //   parseElement
  CHECK_PARSE(parseElement<JsonElement>, "  null  ", 8, JsonElement::null());
  CHECK_PARSE(parseElement<JsonElement>, "  true  ", 8,
              JsonElement::boolean(true));
  CHECK_PARSE(parseElement<JsonElement>, "  false  ", 9,
              JsonElement::boolean(false));
  CHECK_PARSE(parseElement<JsonElement>, "  1234  ", 8,
              JsonElement::number(1234));
  CHECK_PARSE(parseElement<JsonElement>, "  \"\"  ", 6,
              JsonElement::string(""));
  CHECK_PARSE(parseElement<JsonElement>, "  []  ", 6, JsonElement::array(0));
  CHECK_PARSE(parseElement<JsonElement>, "  [\"a\", 2]  ", 12,
              JsonElement::array(2));
  CHECK_PARSE(parseElement<JsonElement>, "  [\"a\", 2  ", -1,
              JsonElement::null());

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
  CHECK_COUNTS("[]", 0, 0, 0, 0, 0, 1, 0, 0, 0);

#define CHECK_DOCPARSE(JSON)                                                   \
  do {                                                                         \
    constexpr std::string_view aJsonStr{JSON};                                 \
    using Builder = DocumentBuilder<Utf8, Utf8>;                               \
    constexpr DocumentInfo aDocInfo =                                 \
        Builder::computeDocInfo(aJsonStr);                                     \
    using DocTy =                                                              \
        Document<aDocInfo.itsNumNumbers, aDocInfo.itsNumChars,                 \
                 aDocInfo.itsNumStrings, aDocInfo.itsNumArrays,                \
                 aDocInfo.itsNumArrayEntries, aDocInfo.itsNumObjects,          \
                 aDocInfo.itsNumObjectProperties>;                             \
    constexpr const std::pair<std::optional<DocTy>, ssize_t> aDocAndLen =      \
        Builder::parseDocument<DocTy>(aJsonStr);                               \
    constexpr ssize_t aParsedLen = aDocAndLen.second;                          \
    static_assert(aDocAndLen.first);                                           \
    static_assert(aParsedLen == static_cast<ssize_t>(aJsonStr.size()));        \
    constexpr DocTy aDoc{*aDocAndLen.first};                                   \
    /*dump(aDoc);*/                                                            \
    std::cout << aDoc << "\n";                                                 \
  } while (false)
  CHECK_DOCPARSE("null");
  CHECK_DOCPARSE("true");
  CHECK_DOCPARSE("false");
  CHECK_DOCPARSE("123");
  CHECK_DOCPARSE("\"123\"");
  CHECK_DOCPARSE("[3,2,1, \"123\",[null,true,false]]");
  CHECK_DOCPARSE("  {\n    \"Test\": false,\n    \"Toast\":true,\n    "
                 "\"Tasty\":[null]\n}  ");
}

int main() {
  static_tests();
#define USE_JSON_STRING(theJson) constexpr std::string_view aJsonSV{theJson};
#include "json_schema.h"
  using Builder = DocumentBuilder<Utf8, Utf8>;
  constexpr DocumentInfo aDocInfo = Builder::computeDocInfo(aJsonSV);
  using DocTy = Document<aDocInfo.itsNumNumbers, aDocInfo.itsNumChars,
                         aDocInfo.itsNumStrings, aDocInfo.itsNumArrays,
                         aDocInfo.itsNumArrayEntries, aDocInfo.itsNumObjects,
                         aDocInfo.itsNumObjectProperties>;
  constexpr auto aDocAndLen = Builder::parseDocument<DocTy>(aJsonSV);
  static_assert(aDocAndLen.first);
  std::cout << "\n" << *aDocAndLen.first << "\n";
}
