#include "constexpr_json/document_builder.h"
#include <cassert>
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
  assert(Utf8::decodeFirst("$").first == 0x24);
  assert(Utf8::decodeFirst("¢").first == 0xa2);
  assert(Utf8::decodeFirst("ह").first == 0x939);
  assert(Utf8::decodeFirst("€").first == 0x20ac);
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