
#include "constexpr_json/document_parser.h"
#include "constexpr_json/dynamic_document.h"
#include "constexpr_json/ext/printing.h"
#include "constexpr_json/impl/document_parser1.h"
#include "constexpr_json/impl/document_parser2.h"
#include "constexpr_json/static_document.h"

#include <cassert>
#include <iostream>
#include <variant>

using namespace cjson;

// static std::ostream &operator<<(std::ostream &theStream,
//                                 const DocumentInfo &theDocInfo) {
//   return theStream << "Nulls: " << theDocInfo.itsNumNulls << "\n"
//                    << "Booleans: " << theDocInfo.itsNumBools << "\n"
//                    << "Numbers: " << theDocInfo.itsNumNumbers << "\n"
//                    << "Characters: " << theDocInfo.itsNumChars << "\n"
//                    << "Strings: " << theDocInfo.itsNumStrings << "\n"
//                    << "Arrays: " << theDocInfo.itsNumArrays << "\n"
//                    << "Array Entries: " << theDocInfo.itsNumArrayEntries <<
//                    "\n"
//                    << "Objects: " << theDocInfo.itsNumObjects << "\n"
//                    << "Object properties: " <<
//                    theDocInfo.itsNumObjectProperties
//                    << "\n";
// }

static std::ostream &operator<<(std::ostream &theStream,
                                const DocumentInterface::EntityRef &theEntity) {
  return Printer<>::template print(theStream, theEntity);
}

static std::ostream &operator<<(std::ostream &theStream,
                                const DocumentInterface &theDoc) {
  return theStream << theDoc.getRoot();
}

// static std::ostream &operator<<(std::ostream &theStream,
//                                 const Entity::KIND &theKind) {
//   switch (theKind) {
//   case Entity::NUL:
//     return theStream << "N";
//   case Entity::BOOL:
//     return theStream << "B";
//   case Entity::NUMBER:
//     return theStream << "D";
//   case Entity::STRING:
//     return theStream << "S";
//   case Entity::ARRAY:
//     return theStream << "A";
//   case Entity::OBJECT:
//     return theStream << "O";
//   }
//   return theStream;
// }

// template <typename DocTy> static void dump(const DocTy &theDoc) {
//   std::cout << "======================================\n"
//             << "#Numbers: " << theDoc.itsNumbers.size() << "\nNumbers: \n";
//   for (const double &aNum : theDoc.itsNumbers)
//     std::cout << aNum << "\n";
//   std::cout << "#Chars: " << theDoc.itsChars.size() << "\nChars: \n";
//   for (const char &aChar : theDoc.itsChars)
//     std::cout << aChar << "\n";
//   std::cout << "#Strings: " << theDoc.itsStrings.size() << "\n";
//   for (const String &aString : theDoc.itsStrings)
//     std::cout << "[" << aString.itsPosition << ": " << aString.itsSize << "]"
//               << "\n";
//   std::cout << "#Arrays: " << theDoc.itsArrays.size() << "\n";
//   for (const Array &aArray : theDoc.itsArrays)
//     std::cout << "[" << aArray.itsPosition << ": " << aArray.itsNumElements
//               << "]"
//               << "\n";
//   std::cout << "#Objects: " << theDoc.itsObjects.size() << "\n";
//   for (const Object &aObject : theDoc.itsObjects)
//     std::cout << "{" << aObject.itsKeysPos << "/" << aObject.itsValuesPos
//               << ": " << aObject.itsNumProperties << "}"
//               << "\n";
//   std::cout << "#Entities: " << theDoc.itsEntities.size() << "\n";
//   for (const Entity &aEntity : theDoc.itsEntities)
//     std::cout << aEntity.itsKind << ":" << aEntity.itsPayload << "\n";
//   std::cout << "======================================\n";
// }

int main() {
  using ErrorHandling = ErrorWillReturnNone;
#define USE_JSON_STRING(theJson) constexpr std::string_view aJsonSV{theJson};
#include "json_schema.h"
  constexpr auto aDocInfoOrError =
      DocumentInfo::compute<Utf8, Utf8, ErrorHandling>(aJsonSV);
  static_assert(!ErrorHandling::isError(aDocInfoOrError));
  constexpr DocumentInfo aDocInfo =
      ErrorHandling::unwrap(aDocInfoOrError).first;
  using DocTy = CJSON_STATIC_DOCTY(aDocInfo);
  { // Test DocumentParser1
    using Parser = DocumentParser1<Utf8, Utf8, ErrorHandling>;
    constexpr auto aDoc = Parser::parseDocument<DocTy>(aJsonSV, aDocInfo);
    static_assert(!ErrorHandling::isError(aDoc));
    static_assert(DocumentInfo::read(aDoc->getRoot()) == aDocInfo);
    std::cout << "\n"
              << DocumentInterfaceImpl<DocTy>{ErrorHandling::unwrap(aDoc)}
              << "\n";
  }
  { // Test DocumentParser2
    using Parser = DocumentParser2<Utf8, Utf8, ErrorHandling>;
    constexpr auto aDoc = Parser::parseDocument<DocTy>(aJsonSV, aDocInfo);
    static_assert(!ErrorHandling::isError(aDoc));
    static_assert(DocumentInfo::read(aDoc->getRoot()) == aDocInfo);
    std::cout << "\n"
              << DocumentInterfaceImpl<DocTy>{ErrorHandling::unwrap(aDoc)}
              << "\n";
  }
  { // Test default DocumentParser
    using Parser = DocumentParser<Utf8, Utf8, ErrorHandling>;
    constexpr auto aDoc = Parser::parseDocument<DocTy>(aJsonSV, aDocInfo);
    static_assert(!ErrorHandling::isError(aDoc));
    static_assert(DocumentInfo::read(aDoc->getRoot()) == aDocInfo);
    std::cout << "\n"
              << DocumentInterfaceImpl<DocTy>{ErrorHandling::unwrap(aDoc)}
              << "\n";
  }
  { // Test default DocumentParser with DynamicDocument (parsing at runtime)
    using Parser = DocumentParser<Utf8, Utf8, ErrorHandling>;
    const auto aDoc = Parser::parseDocument<DynamicDocument>(aJsonSV, aDocInfo);
    assert(!ErrorHandling::isError(aDoc));
    std::cout << "\n" << ErrorHandling::unwrap(aDoc) << "\n";
  }
}
