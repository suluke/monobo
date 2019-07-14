#include "constexpr_json/document_builder.h"
#include <cassert>
#include <cmath>
#include <iostream>
#include <variant>

using namespace cjson;

std::ostream &operator<<(std::ostream &theStream,
                         const DocumentInfo &theDocInfo) {
  return theStream << "Doubles: " << theDocInfo.itsNumDoubles << "\n"
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

int main() {
  // Test unicode decoder
  assert(Utf8::decodeFirst("$").first == 0x24);
  assert(Utf8::decodeFirst("¢").first == 0xa2);
  assert(Utf8::decodeFirst("ह").first == 0x939);
  assert(Utf8::decodeFirst("€").first == 0x20ac);

  // Test parsing procedures
  using p = parsing<Utf8>;
  //   readWhitespace
  assert(p::readWhitespace("").empty());
  assert(p::readWhitespace("     ").size() == 5);
  assert(p::readWhitespace("     abc").size() == 5);
  assert(p::readWhitespace(std::begin({(char)0xa, 'a'})).size() == 1);
  assert(p::readWhitespace(std::begin({(char)0xd, 'a'})).size() == 1);
  assert(p::readWhitespace(std::begin({(char)0x9, 'a'})).size() == 1);
  assert(p::readWhitespace(std::begin({(char)0x20, 'a'})).size() == 1);
  //   readDigits
  assert(p::readDigits("").empty());
  assert(p::readDigits(" 123abc").empty());
  assert(p::readDigits("123abc").size() == 3);
  //   parseInt
  assert(p::parseInteger("").second <= 0);
  assert(p::parseInteger("0123").second == 1);
  assert(p::parseInteger("-1").first == -1.);
  assert(p::parseInteger("-1").second == 2);
  assert(p::parseInteger("-0123").first == 0.);
  assert(p::parseInteger("-0123").second == 2);
  assert(std::signbit(p::parseInteger("-0123").first));
  assert(p::parseInteger("1").first == 1.);
  assert(p::parseInteger("1").second == 1);
  assert(p::parseInteger("10").first == 10.);
  assert(p::parseInteger("10").second == 2);
  assert(p::parseInteger("-1").first == -1.);
  assert(p::parseInteger("-1").second == 2);
  assert(p::parseInteger("-10").first == -10.);
  assert(p::parseInteger("-10").second == 3);
  assert(p::parseInteger("-10abc").first == -10.);
  assert(p::parseInteger("-10abc").second == 3);
  //   readFraction
  assert(p::readFraction("") == "");
  assert(p::readFraction(".") == "");
  assert(p::readFraction(".1") == ".1");
  //   parseExponent
  assert(p::parseExponent("e0").first == 0);
  assert(p::parseExponent("e0").second == 2);
  assert(p::parseExponent("E0").first == 0);
  assert(p::parseExponent("E0").second == 2);
  assert(p::parseExponent("E1").first == 1);
  assert(p::parseExponent("E1").second == 2);
  assert(p::parseExponent("E+1").first == 1);
  assert(p::parseExponent("E+1").second == 3);
  assert(p::parseExponent("e-3").first == -3);
  assert(p::parseExponent("e-3").second == 3);
  //   parseNumber
  assert(p::parseNumber("0").first == 0);
  assert(p::parseNumber("0").second == 1);
  assert(p::parseNumber("01").first == 0);
  assert(p::parseNumber("01").second == 1);
  assert(p::parseNumber("1").first == 1.);
  assert(p::parseNumber("1234").first == 1234.);
  assert(p::parseNumber("01").second == 1);
  assert(p::parseNumber("-1").first == -1.);
  assert(p::parseNumber("-1").second == 2);
  assert(p::parseNumber("-1.2").first == -1.2);
  assert(p::parseNumber("-1.2").second == 4);
  assert(p::parseNumber("-1.0e3").first == -100.);
  assert(p::parseNumber("-1.0e3").second == 6);
  assert(p::parseNumber("-1.0e-3").first == -0.001);
  assert(p::parseNumber("-1.0e-3").second == 7);
  assert(p::parseNumber("-1.0e-03").first == -0.001);
  assert(p::parseNumber("-1.0e-03").second == 8);
  assert(p::parseNumber("2]").first == 2);
  assert(p::parseNumber("2]").second == 1);
  //   parseNull
  assert(p::parseNull("abcdee") <= 0);
  assert(p::parseNull("null") == 4);
  assert(p::parseNull("nullabcd") == 4);
  //   parseBool
  assert(p::parseBool("abcdee").second <= 0);
  assert(p::parseBool("true").first);
  assert(p::parseBool("true").second == 4);
  assert(!p::parseBool("false").first);
  assert(p::parseBool("false").second == 5);
  assert(p::parseBool("trueabc").first);
  assert(p::parseBool("trueabc").second == 4);
  assert(!p::parseBool("falseabc").first);
  assert(p::parseBool("falseabc").second == 5);
  //   readString
  assert(p::readString("").empty());
  assert(p::readString(R"~("\")~").empty());
  assert(p::readString(R"~("")~").size() == 2);
  assert(p::readString(R"~("\n")~").size() == 4);
  assert(p::readString(R"~("\uabcd")~").size() == 8);
  // Parsing elements
  struct JsonElement {
    Entity::KIND itsType = Entity::NUL;
    bool itsBoolVal = false;
    double itsNumberVal = 0.;
    std::string_view itsStringVal;
    size_t itsArrayLen = 0;
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
  };
  //   parseArray
  using namespace std::literals;
#define CHECK_PARSE(FN, STR, EXPECTED_LEN, EXPECTED_ELM)                       \
  do {                                                                         \
    constexpr std::pair<JsonElement, ssize_t> aParsed =                        \
        p::FN<JsonElement>(STR##sv);                                           \
    constexpr JsonElement aElem = aParsed.first;                               \
    constexpr ssize_t aElemLength = aParsed.second;                            \
    constexpr JsonElement aExpectedElm = EXPECTED_ELM;                         \
    static_assert(EXPECTED_LEN == aElemLength);                                \
    static_assert(aExpectedElm == aElem);                                      \
  } while (false)
  CHECK_PARSE(parseArray, "[\"a\", 2]  ", 8, JsonElement::array(2));
  CHECK_PARSE(parseArray, "[\"a\", 2  ", -1, JsonElement::null());
  //   parseElement
  CHECK_PARSE(parseElement, "  null  ", 8, JsonElement::null());
  CHECK_PARSE(parseElement, "  true  ", 8, JsonElement::boolean(true));
  CHECK_PARSE(parseElement, "  false  ", 9, JsonElement::boolean(false));
  CHECK_PARSE(parseElement, "  1234  ", 8, JsonElement::number(1234));
  CHECK_PARSE(parseElement, "  \"\"  ", 6, JsonElement::string("\"\""));
  CHECK_PARSE(parseElement, "  []  ", 6, JsonElement::array(0));
  CHECK_PARSE(parseElement, "  [\"a\", 2]  ", 12, JsonElement::array(2));
  CHECK_PARSE(parseElement, "  [\"a\", 2  ", -1, JsonElement::null());

#define USE_JSON_STRING(theJson) constexpr const char aJsonStr[] = theJson;
#include "json_schema.h"
  // std::cout << aJsonStr;
  constexpr DocumentInfo aDocInfo = computeDocInfo(aJsonStr);
  std::cout << aDocInfo;
  Document<aDocInfo.itsNumDoubles, aDocInfo.itsNumChars, aDocInfo.itsNumStrings,
           aDocInfo.itsNumArrays, aDocInfo.itsNumArrayEntries,
           aDocInfo.itsNumObjects, aDocInfo.itsNumObjectProperties>
      aDoc;
}