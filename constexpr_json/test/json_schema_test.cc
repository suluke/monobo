#include "constexpr_json/document_builder.h"
#include <cassert>
#include <cmath>
#include <iostream>
#include <variant>

using namespace cjson;

std::ostream &operator<<(std::ostream &theStream,
                         const DocumentInfo &theDocInfo) {
  return theStream << "Nulls: " << theDocInfo.itsNumNulls << "\n"
                   << "Booleans: " << theDocInfo.itsNumBools << "\n"
                   << "Doubles: " << theDocInfo.itsNumDoubles << "\n"
                   << "Characters: " << theDocInfo.itsNumChars << "\n"
                   << "Strings: " << theDocInfo.itsNumStrings << "\n"
                   << "Arrays: " << theDocInfo.itsNumArrays << "\n"
                   << "Array Entries: " << theDocInfo.itsNumArrayEntries << "\n"
                   << "Objects: " << theDocInfo.itsNumObjects << "\n"
                   << "Object properties: " << theDocInfo.itsNumObjectProperties
                   << "\n";
}

std::ostream &operator<<(std::ostream &theStream, const EntityRef &theEntity) {
  switch (theEntity.getType()) {
  case Entity::BOOLEAN:
    return theStream << (theEntity.toBool() ? "true" : "false");
  case Entity::DOUBLE:
    return theStream << theEntity.toDouble();
  case Entity::NUL:
    return theStream << "null";
  case Entity::ARRAY:
    return theStream;
  case Entity::OBJECT:
    return theStream;
  case Entity::STRING:
    return theStream;
  }
  return theStream;
}

std::ostream &operator<<(std::ostream &theStream, const DocumentBase &theDoc) {
  return theStream << theDoc.getRoot();
}

struct JsonElement {
  Entity::KIND itsType = Entity::NUL;
  bool itsBoolVal = false;
  double itsNumberVal = 0.;
  std::string_view itsStringVal;
  size_t itsArrayLen = 0;
  size_t itsNumProperties = 0;
  constexpr void setBool(bool theBool) {
    itsType = Entity::BOOLEAN;
    itsBoolVal = theBool;
  }
  constexpr void setNumber(double theNum) {
    itsType = Entity::DOUBLE;
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
    case Entity::BOOLEAN:
      return theOther.itsBoolVal == itsBoolVal;
    case Entity::DOUBLE:
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

int main() {
  // Test unicode decoder
  CHECK_UTF8_DECODE("$", 0x24);
  CHECK_UTF8_DECODE("¢", 0xa2);
  CHECK_UTF8_DECODE("ह", 0x939);
  CHECK_UTF8_DECODE("€", 0x20ac);

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
              JsonElement::string("\"\""));
  CHECK_PARSE(parseElement<JsonElement>, "  []  ", 6, JsonElement::array(0));
  CHECK_PARSE(parseElement<JsonElement>, "  [\"a\", 2]  ", 12,
              JsonElement::array(2));
  CHECK_PARSE(parseElement<JsonElement>, "  [\"a\", 2  ", -1,
              JsonElement::null());

#define USE_JSON_STRING(theJson) constexpr std::string_view aJsonSV{theJson};
#include "json_schema.h"
  // std::cout << aJsonStr;
  constexpr DocumentInfo aDocInfo = computeDocInfo<Utf8>(aJsonSV);
  std::cout << aDocInfo;
  Document<aDocInfo.itsNumDoubles, aDocInfo.itsNumChars, aDocInfo.itsNumStrings,
           aDocInfo.itsNumArrays, aDocInfo.itsNumArrayEntries,
           aDocInfo.itsNumObjects, aDocInfo.itsNumObjectProperties>
      aDoc;
  std::ignore = aDoc;
}
