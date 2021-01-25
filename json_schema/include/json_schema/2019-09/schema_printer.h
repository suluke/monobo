#ifndef JSON_SCHEMA_SCHEMA_PRINTER_H
#define JSON_SCHEMA_SCHEMA_PRINTER_H

#include <iomanip>
#include <iostream>

#include "constexpr_json/ext/printing.h"
#include "json_schema/2019-09/model/applicator.h"
#include "json_schema/2019-09/model/core.h"
#include "json_schema/2019-09/model/format.h"
#include "json_schema/2019-09/model/validation.h"

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

  static constexpr inline int W_COL1 = 18;

  static void printObject(std::ostream &theOS, const SchemaObject &theObj,
                          const unsigned theDepth = 0) {
    theOS << std::left << std::setfill(' ');
    printObjectCore(theOS, theObj, theDepth);
    printObjectApplicator(theOS, theObj, theDepth);
    printObjectValidation(theOS, theObj, theDepth);
    printObjectFormat(theOS, theObj, theDepth);
  }

  static void printSchema(std::ostream &theOS,
                          const std::variant<bool, SchemaObject> &theSchema,
                          const int theDepth,
                          const bool theFreestanding = false) {
    if (std::holds_alternative<bool>(theSchema)) {
      if (theFreestanding)
        theOS << indent(theDepth);
      else
        theOS << " ";
      theOS << (std::get<bool>(theSchema) ? "true" : "false") << "\n";
    } else {
      if (!theFreestanding)
        theOS << "\n";
      printObject(theOS, std::get<SchemaObject>(theSchema), theDepth + 1);
    }
  }

  static void printObjectCore(std::ostream &theOS, const SchemaObject &theObj,
                              const unsigned theDepth) {
    const auto &aCore = theObj.template getSection<SchemaCore>();
    theOS << indent(theDepth) << "# Core\n";
    theOS << indent(theDepth) << std::setw(W_COL1)
          << "$id:" << aCore.getId().toString() << "\n";
    theOS << indent(theDepth) << std::setw(W_COL1)
          << "$schema:" << aCore.getSchema().toString() << "\n";
    theOS << indent(theDepth) << std::setw(W_COL1)
          << "$anchor:" << aCore.getAnchor() << "\n";
    theOS << indent(theDepth) << std::setw(W_COL1)
          << "$ref:" << aCore.getRef().toString() << "\n";
    theOS << indent(theDepth) << std::setw(W_COL1)
          << "$recursiveRef:" << aCore.getRecursiveRef().toString() << "\n";
    theOS << indent(theDepth) << std::setw(W_COL1) << "$recursiveAnchor:"
          << (aCore.getRecursiveAnchor() ? "true" : "false") << "\n";
    const auto aVocabDict = aCore.getVocabulary().toDict();
    theOS << indent(theDepth) << std::setw(W_COL1)
          << "$vocabulary:" << aVocabDict.size() << "\n";
    for (const auto [aKey, aValue] : aVocabDict) {
      theOS << indent(theDepth + 1) << aKey << ": "
            << (aValue ? "true" : "false") << "\n";
    }
    theOS << indent(theDepth) << std::setw(W_COL1)
          << "$comment:" << aCore.getComment() << "\n";
    const auto aDefsDict = aCore.getDefs().toDict();
    theOS << indent(theDepth) << std::setw(W_COL1)
          << "$defs:" << aDefsDict.size() << "\n";
    for (const auto [aKey, aValue] : aDefsDict) {
      theOS << indent(theDepth + 1) << aKey << ":";
      printSchema(theOS, aValue, theDepth + 1);
    }
  }

  static void printObjectApplicator(std::ostream &theOS,
                                    const SchemaObject &theObj,
                                    const unsigned theDepth) {
    const auto &aApplicator = theObj.template getSection<SchemaApplicator>();
    theOS << indent(theDepth) << "# Applicator\n";
    const auto aPrintSchema =
        [&theOS, theDepth](const std::variant<bool, SchemaObject> &theSchema) {
          printSchema(theOS, theSchema, theDepth + 1);
        };
    theOS << indent(theDepth) << "additionalItems:";
    aPrintSchema(aApplicator.getAdditionalItems());
    theOS << indent(theDepth) << "unevaluatedItems:";
    aPrintSchema(aApplicator.getUnevaluatedItems());
    theOS << indent(theDepth) << "items:\n";
    for (const auto aSchema : aApplicator.getItems())
      printSchema(theOS, aSchema, theDepth + 1, true);
    theOS << indent(theDepth) << "contains:";
    aPrintSchema(aApplicator.getContains());
    theOS << indent(theDepth) << "additionalProperties:";
    aPrintSchema(aApplicator.getAdditionalProperties());
    theOS << indent(theDepth) << "unevaluatedProperties:";
    aPrintSchema(aApplicator.getUnevaluatedProperties());
    theOS << indent(theDepth) << "properties:\n";
    for (const auto [aKey, aSchema] : aApplicator.getProperties()) {
      theOS << indent(theDepth + 1) << aKey << ":";
      aPrintSchema(aSchema);
    }
    theOS << indent(theDepth) << "patternProperties:\n";
    for (const auto [aKey, aSchema] : aApplicator.getPatternProperties()) {
      theOS << indent(theDepth + 1) << aKey << ":";
      aPrintSchema(aSchema);
    }
    theOS << indent(theDepth) << "dependentSchemas:\n";
    for (const auto [aKey, aSchema] : aApplicator.getDependentSchemas()) {
      theOS << indent(theDepth + 1) << aKey << ":";
      aPrintSchema(aSchema);
    }
    theOS << indent(theDepth) << "propertyNames:";
    aPrintSchema(aApplicator.getPropertyNames());
    theOS << indent(theDepth) << "if:";
    aPrintSchema(aApplicator.getIf());
    theOS << indent(theDepth) << "then:";
    aPrintSchema(aApplicator.getThen());
    theOS << indent(theDepth) << "else:";
    aPrintSchema(aApplicator.getElse());
    theOS << indent(theDepth) << "allOf:\n";
    for (const auto aSchema : aApplicator.getAllOf())
      printSchema(theOS, aSchema, theDepth + 1, true);
    theOS << indent(theDepth) << "anyOf:\n";
    for (const auto aSchema : aApplicator.getAnyOf())
      printSchema(theOS, aSchema, theDepth + 1, true);
    theOS << indent(theDepth) << "oneOf:\n";
    for (const auto aSchema : aApplicator.getOneOf())
      printSchema(theOS, aSchema, theDepth + 1, true);
    theOS << indent(theDepth) << "not:";
    aPrintSchema(aApplicator.getNot());
  }

  static constexpr const char *toString(const Types theType) {
    switch (theType) {
    case Types::ARRAY:
      return "array";
    case Types::BOOLEAN:
      return "boolean";
    case Types::INTEGER:
      return "integer";
    case Types::NUL:
      return "null";
    case Types::NUMBER:
      return "number";
    case Types::OBJECT:
      return "object";
    case Types::STRING:
      return "string";
    }
    throw "Invalid type";
  }

  static void printObjectValidation(std::ostream &theOS,
                                    const SchemaObject &theObj,
                                    const unsigned theDepth) {
    const auto &aValidation = theObj.template getSection<SchemaValidation>();
    theOS << indent(theDepth) << "# Validation\n";
    theOS << indent(theDepth) << std::setw(W_COL1)
          << "multipleOf:" << aValidation.getMultipleOf() << "\n";
    theOS << indent(theDepth) << std::setw(W_COL1)
          << "maximum:" << aValidation.getMaximum() << "\n";
    theOS << indent(theDepth) << std::setw(W_COL1)
          << "exclusiveMaximum:" << aValidation.getExclusiveMaximum() << "\n";
    theOS << indent(theDepth) << std::setw(W_COL1)
          << "minimum:" << aValidation.getMinimum() << "\n";
    theOS << indent(theDepth) << std::setw(W_COL1)
          << "exclusiveMinimum:" << aValidation.getExclusiveMinimum() << "\n";
    theOS << indent(theDepth) << std::setw(W_COL1)
          << "maxLength:" << aValidation.getMaxLength() << "\n";
    theOS << indent(theDepth) << std::setw(W_COL1)
          << "minLength:" << aValidation.getMinLength() << "\n";
    theOS << indent(theDepth) << std::setw(W_COL1)
          << "maxItems:" << aValidation.getMaxItems() << "\n";
    theOS << indent(theDepth) << std::setw(W_COL1)
          << "minItems:" << aValidation.getMinItems() << "\n";
    theOS << indent(theDepth) << std::setw(W_COL1)
          << "maxContains:" << aValidation.getMaxContains() << "\n";
    theOS << indent(theDepth) << std::setw(W_COL1)
          << "minContains:" << aValidation.getMinContains() << "\n";
    theOS << indent(theDepth) << std::setw(W_COL1)
          << "maxProperties:" << aValidation.getMaxProperties() << "\n";
    theOS << indent(theDepth) << std::setw(W_COL1)
          << "minProperties:" << aValidation.getMinProperties() << "\n";
    theOS << indent(theDepth) << std::setw(W_COL1)
          << "pattern:" << aValidation.getPattern() << "\n";
    theOS << indent(theDepth) << "required:\n";
    for (const auto aValue : aValidation.getRequired()) {
      theOS << indent(theDepth + 1) << aValue << "\n";
    }
    theOS << indent(theDepth) << "dependentRequired:\n";
    for (const auto [aKey, aStringList] : aValidation.getDependentRequired()) {
      theOS << indent(theDepth + 1) << aKey << ":\n";
      for (const auto aValue : aStringList) {
        theOS << indent(theDepth + 2) << aValue << "\n";
      }
    }
    theOS << indent(theDepth) << "const: ";
    const auto aConstMaybe = aValidation.getConst();
    if (aConstMaybe) {
      cjson::Printer aJsonPrinter;
      aJsonPrinter.print(theOS, *aConstMaybe) << "\n";
    } else {
      theOS << "\n";
    }
    theOS << indent(theDepth) << "enum:\n";
    cjson::Printer aJsonPrinter;
    for (const auto aJson : aValidation.getEnum()) {
      theOS << indent(theDepth + 1);
      aJsonPrinter.print(theOS, aJson) << "\n";
    }
    theOS << indent(theDepth) << "type:\n";
    for (const Types aType : aValidation.getType())
      theOS << indent(theDepth + 1) << toString(aType) << "\n";
  }

  static void printObjectFormat(std::ostream &theOS, const SchemaObject &theObj,
                                const unsigned theDepth) {
    const auto &aFormat = theObj.template getSection<SchemaFormat>();
    theOS << indent(theDepth) << "# Format\n";
    theOS << indent(theDepth) << std::setw(W_COL1)
          << "format: " << aFormat.getFormat() << "\n";
  }

  const SchemaObject &itsSchemaObject;
};
} // namespace json_schema
#endif // JSON_SCHEMA_SCHEMA_PRINTER_H
