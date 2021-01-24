#ifndef JSON_SCHEMA_SCHEMA_PRINTER_H
#define JSON_SCHEMA_SCHEMA_PRINTER_H

#include <iomanip>
#include <iostream>

#include "json_schema/2019-09/model/core.h"

namespace json_schema {

struct SchemaPrinterBase {
  friend std::ostream &operator<<(std::ostream &theOS,
                                  const SchemaPrinterBase &thePrinter) {
    thePrinter.print(theOS);
    return theOS;
  }
  virtual void print(std::ostream &theOS) const = 0;
};

template <typename SchemaObject>
class SchemaPrinter : public SchemaPrinterBase {
public:
  SchemaPrinter(const SchemaObject &theSchemaObject)
      : itsSchemaObject(theSchemaObject) {}
  void print(std::ostream &theOS) const override {
    printObject(theOS, itsSchemaObject);
  }

private:
  struct Indenter {
    unsigned itsDepth;
    friend inline std::ostream &operator<<(std::ostream &theOS,
                                           const Indenter &theIndenter) {
      if (theIndenter.itsDepth == 0)
        return theOS;
      return theOS << std::setw(2 * theIndenter.itsDepth) << ' ';
    }
  };

  static Indenter indent(const unsigned theDepth) { return Indenter{theDepth}; }

  static void printObject(std::ostream &theOS, const SchemaObject &theObj,
                          const unsigned theDepth = 0) {
    printObjectCore(theOS, theObj, theDepth);
  }

  static void printObjectCore(std::ostream &theOS, const SchemaObject &theObj,
                              const unsigned theDepth) {
    const auto &aCore = theObj.template getSection<SchemaCore>();
    constexpr int aKWWidth = 18;
    theOS << std::left << std::setfill(' ');
    theOS << indent(theDepth) << "# Core\n";
    theOS << indent(theDepth) << std::setw(aKWWidth)
          << "$id:" << aCore.getId().toString() << "\n";
    theOS << indent(theDepth) << std::setw(aKWWidth)
          << "$schema:" << aCore.getSchema().toString() << "\n";
    theOS << indent(theDepth) << std::setw(aKWWidth)
          << "$anchor:" << aCore.getAnchor() << "\n";
    theOS << indent(theDepth) << std::setw(aKWWidth)
          << "$ref:" << aCore.getRef().toString() << "\n";
    theOS << indent(theDepth) << std::setw(aKWWidth)
          << "$recursiveRef:" << aCore.getRecursiveRef().toString() << "\n";
    theOS << indent(theDepth) << std::setw(aKWWidth) << "$recursiveAnchor:"
          << (aCore.getRecursiveAnchor() ? "true" : "false") << "\n";
    const auto aVocabDict = aCore.getVocabulary().toDict();
    theOS << indent(theDepth) << std::setw(aKWWidth)
          << "$vocabulary:" << aVocabDict.size() << "\n";
    for (const auto [aKey, aValue] : aVocabDict) {
      theOS << indent(theDepth + 1) << aKey << ": "
            << (aValue ? "true" : "false") << "\n";
    }
    theOS << indent(theDepth) << std::setw(aKWWidth)
          << "$comment:" << aCore.getComment() << "\n";
    const auto aDefsDict = aCore.getDefs().toDict();
    theOS << indent(theDepth) << std::setw(aKWWidth)
          << "$defs:" << aDefsDict.size() << "\n";
    for (const auto [aKey, aValue] : aDefsDict) {
      theOS << indent(theDepth + 1) << aKey << ":";
      if (std::holds_alternative<bool>(aValue)) {
        theOS << " " << (std::get<bool>(aValue) ? "true" : "false") << "\n";
      } else {
        theOS << "\n";
        printObject(theOS, std::get<SchemaObject>(aValue), theDepth + 2);
      }
    }
  }

  const SchemaObject &itsSchemaObject;
};
} // namespace json_schema
#endif // JSON_SCHEMA_SCHEMA_PRINTER_H
