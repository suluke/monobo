#include "constexpr_json/document_builder.h"
#include <cassert>
#include <cmath>
#include <iostream>

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