
#include "constexpr_json/document_builder.h"
#include "constexpr_json/dynamic_document.h"
#include "constexpr_json/impl/document_builder1.h"
#include "constexpr_json/impl/document_builder2.h"
#include "constexpr_json/static_document.h"
#include "constexpr_json/ext/printing.h"

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
//                    << "Array Entries: " << theDocInfo.itsNumArrayEntries << "\n"
//                    << "Objects: " << theDocInfo.itsNumObjects << "\n"
//                    << "Object properties: " << theDocInfo.itsNumObjectProperties
//                    << "\n";
// }

static std::ostream &operator<<(std::ostream &theStream,
                                const DocumentInterface::EntityRef &theEntity) {
  return print(theStream, theEntity);
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
  using DocTy =
      StaticDocument<aDocInfo.itsNumNumbers, aDocInfo.itsNumChars,
                     aDocInfo.itsNumStrings, aDocInfo.itsNumArrays,
                     aDocInfo.itsNumArrayEntries, aDocInfo.itsNumObjects,
                     aDocInfo.itsNumObjectProperties>;
  { // Test DocumentBuilder1
    using Builder = DocumentBuilder1<Utf8, Utf8, ErrorHandling>;
    constexpr auto aDoc = Builder::parseDocument<DocTy>(aJsonSV, aDocInfo);
    static_assert(!ErrorHandling::isError(aDoc));
    std::cout << "\n" << DocumentInterfaceImpl<DocTy>{ErrorHandling::unwrap(aDoc)} << "\n";
  }
  { // Test DocumentBuilder2
    using Builder = DocumentBuilder2<Utf8, Utf8, ErrorHandling>;
    constexpr auto aDoc = Builder::parseDocument<DocTy>(aJsonSV, aDocInfo);
    static_assert(!ErrorHandling::isError(aDoc));
    std::cout << "\n" << DocumentInterfaceImpl<DocTy>{ErrorHandling::unwrap(aDoc)} << "\n";
  }
  { // Test default DocumentBuilder
    using Builder = DocumentBuilder<Utf8, Utf8, ErrorHandling>;
    constexpr auto aDoc = Builder::parseDocument<DocTy>(aJsonSV, aDocInfo);
    static_assert(!ErrorHandling::isError(aDoc));
    std::cout << "\n" << DocumentInterfaceImpl<DocTy>{ErrorHandling::unwrap(aDoc)} << "\n";
  }
  { // Test default DocumentBuilder with DynamicDocument (parsing at runtime)
    using Builder = DocumentBuilder<Utf8, Utf8, ErrorHandling>;
    const auto aDoc =
        Builder::parseDocument<DynamicDocument>(aJsonSV, aDocInfo);
    assert(!ErrorHandling::isError(aDoc));
    std::cout << "\n" << ErrorHandling::unwrap(aDoc) << "\n";
  }
}
