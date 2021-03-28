#ifndef JSON_SCHEMA_SCHEMA_PRINTER_H
#define JSON_SCHEMA_SCHEMA_PRINTER_H

#include <iomanip>
#include <iostream>

#include "constexpr_json/ext/printing.h"
#include "json_schema/2019-09/model/applicator.h"
#include "json_schema/2019-09/model/content.h"
#include "json_schema/2019-09/model/core.h"
#include "json_schema/2019-09/model/format.h"
#include "json_schema/2019-09/model/metadata.h"
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

  //============ BEGIN Hacks to avoid code duplication ============
  // This printer is divided into one printX function per schema section.
  // Each section will be introduced with a headline followed by the actual
  // section content. If no content is printed in the absence of any set
  // values, however, the headline should be omitted. To avoid writing code
  // that checks if any value is set, we determine if any content is written
  // to a dummy printer instead before writing to the actual output stream.
  // The following implements the dummy printer infrastructure.
  struct WriteCheckStream {
    bool itsWasWritten = false;
    template <typename V> WriteCheckStream &operator<<(V &&) {
      itsWasWritten = true;
      return *this;
    }
  };
  template <typename T> static bool hasOutput(T &&theStreamWriter) {
    WriteCheckStream aWriteChecker;
    theStreamWriter(aWriteChecker);
    return aWriteChecker.itsWasWritten;
  }
  struct DummyPrinter {
    template <typename... Args> DummyPrinter &print(Args &&...) {
      return *this;
    }
    template <typename T> DummyPrinter &operator<<(T &&) { return *this; }
  };

  template <typename OS>
  using JsonPrinter =
      std::conditional_t<std::is_convertible_v<OS, std::ostream>,
                         cjson::Printer<>, DummyPrinter>;
  //============= END Hacks to avoid code duplication =============

  template <typename OS>
  static void printObject(OS &theOS, const SchemaObject &theObj,
                          const unsigned theDepth = 0) {
    theOS << std::left << std::setfill(' ');
    printObjectCore(theOS, theObj, theDepth);
    printObjectApplicator(theOS, theObj, theDepth);
    printObjectValidation(theOS, theObj, theDepth);
    printObjectFormat(theOS, theObj, theDepth);
    printObjectMetadata(theOS, theObj, theDepth);
    printObjectContent(theOS, theObj, theDepth);
  }

  template <typename OS>
  static void
  printSchema(OS &theOS, const std::variant<bool, SchemaObject> &theSchema,
              const int theDepth, const bool theFreestanding = false) {
    if (std::holds_alternative<bool>(theSchema)) {
      if (theFreestanding)
        theOS << indent(theDepth);
      else
        theOS << " ";
      theOS << (std::get<bool>(theSchema) ? "true" : "false") << "\n";
    } else {
      if (!theFreestanding)
        theOS << "\n";
      theOS << indent(theDepth) << "## SCHEMA\n";
      printObject(theOS, std::get<SchemaObject>(theSchema), theDepth + 1);
    }
  }

  template <typename OS>
  static void printObjectCore(OS &theOS, const SchemaObject &theObj,
                              const unsigned theDepth) {
    using namespace std::literals;
    const auto &aCore = theObj.template getSection<SchemaCore>();
    const auto printTo = [aCore, theDepth](auto &theOS) {
      if (const auto aId = aCore.getId().toString()) {
        theOS << indent(theDepth) << std::setw(W_COL1) << "$id:" << *aId
              << "\n";
      }
      if (const auto aSchema = aCore.getSchema().toString()) {
        theOS << indent(theDepth) << std::setw(W_COL1) << "$schema:" << *aSchema
              << "\n";
      }
      if (const auto aAnchor = aCore.getAnchor()) {
        theOS << indent(theDepth) << std::setw(W_COL1) << "$anchor:" << *aAnchor
              << "\n";
      }
      if (const auto aRef = aCore.getRef().toString()) {
        theOS << indent(theDepth) << std::setw(W_COL1) << "$ref:" << *aRef
              << "\n";
      }
      if (const auto aRecRef = aCore.getRecursiveRef().toString()) {
        theOS << indent(theDepth) << std::setw(W_COL1)
              << "$recursiveRef:" << *aRecRef << "\n";
      }
      if (aCore.getRecursiveAnchor().has_value()) {
        theOS << indent(theDepth) << std::setw(W_COL1) << "$recursiveAnchor:"
              << (aCore.getRecursiveAnchor().value() ? "true" : "false")
              << "\n";
      }
      if (const auto aVocabDict = aCore.getVocabulary().toDict()) {
        theOS << indent(theDepth) << std::setw(W_COL1)
              << "$vocabulary:" << aVocabDict->size() << "\n";
        for (const auto [aKey, aValue] : *aVocabDict) {
          theOS << indent(theDepth + 1) << aKey << ": "
                << (aValue ? "true" : "false") << "\n";
        }
      }
      if (const auto aComment = aCore.getComment()) {
        theOS << indent(theDepth) << std::setw(W_COL1)
              << "$comment:" << *aComment << "\n";
      }
      if (const auto aDefsDict = aCore.getDefs().toDict()) {
        theOS << indent(theDepth) << std::setw(W_COL1)
              << "$defs:" << aDefsDict->size() << "\n";
        for (const auto [aKey, aValue] : *aDefsDict) {
          theOS << indent(theDepth + 1) << aKey << ":";
          printSchema(theOS, aValue, theDepth + 1);
        }
      }
    };
    if (hasOutput(printTo)) {
      theOS << indent(theDepth) << "# Core\n";
      printTo(theOS);
    }
  }

  template <typename OS>
  static void printObjectApplicator(OS &theOS, const SchemaObject &theObj,
                                    const unsigned theDepth) {
    const auto &aApplicator = theObj.template getSection<SchemaApplicator>();
    const auto printTo = [aApplicator, theDepth](auto &theOS) {
      const auto aPrintSchema =
          [&theOS,
           theDepth](const std::optional<std::variant<bool, SchemaObject>>
                         &theSchema) {
            if (theSchema)
              printSchema(theOS, *theSchema, theDepth + 1);
            else
              theOS << " none\n";
          };
      if (const auto aAdditionalItems = aApplicator.getAdditionalItems()) {
        theOS << indent(theDepth) << std::setw(W_COL1) << "additionalItems:";
        aPrintSchema(aAdditionalItems);
      }
      if (const auto aUnevaluatedItems = aApplicator.getUnevaluatedItems()) {
        theOS << indent(theDepth) << std::setw(W_COL1) << "unevaluatedItems:";
        aPrintSchema(aUnevaluatedItems);
      }
      if (const auto aItems = aApplicator.getItems()) {
        theOS << indent(theDepth) << "items:\n";
        for (const auto aSchema : *aItems)
          printSchema(theOS, aSchema, theDepth + 1, true);
      }
      if (const auto aContains = aApplicator.getContains()) {
        theOS << indent(theDepth) << std::setw(W_COL1) << "contains:";
        aPrintSchema(aContains);
      }
      if (const auto aAdditionalProperties =
              aApplicator.getAdditionalProperties()) {
        theOS << indent(theDepth) << std::setw(W_COL1)
              << "additionalProperties:";
        aPrintSchema(aAdditionalProperties);
      }
      if (const auto aUnevaluatedProperties =
              aApplicator.getUnevaluatedProperties()) {
        theOS << indent(theDepth) << std::setw(W_COL1)
              << "unevaluatedProperties:";
        aPrintSchema(aUnevaluatedProperties);
      }
      if (const auto aProperties = aApplicator.getProperties()) {
        theOS << indent(theDepth) << "properties:\n";
        for (const auto [aKey, aSchema] : *aProperties) {
          theOS << indent(theDepth + 1) << aKey << ":";
          aPrintSchema(aSchema);
        }
      }
      if (const auto aPatternProperties = aApplicator.getPatternProperties()) {
        theOS << indent(theDepth) << "patternProperties:\n";
        for (const auto [aKey, aSchema] : *aPatternProperties) {
          theOS << indent(theDepth + 1) << aKey << ":";
          aPrintSchema(aSchema);
        }
      }
      if (const auto aDependentSchemas = aApplicator.getDependentSchemas()) {
        theOS << indent(theDepth) << "dependentSchemas:\n";
        for (const auto [aKey, aSchema] : *aDependentSchemas) {
          theOS << indent(theDepth + 1) << aKey << ":";
          aPrintSchema(aSchema);
        }
      }
      if (const auto aPropertyNames = aApplicator.getPropertyNames()) {
        theOS << indent(theDepth) << std::setw(W_COL1) << "propertyNames:";
        aPrintSchema(aPropertyNames);
      }
      if (const auto aIf = aApplicator.getIf()) {
        theOS << indent(theDepth) << std::setw(W_COL1) << "if:";
        aPrintSchema(aIf);
      }
      if (const auto aThen = aApplicator.getThen()) {
        theOS << indent(theDepth) << std::setw(W_COL1) << "then:";
        aPrintSchema(aThen);
      }
      if (const auto aElse = aApplicator.getElse()) {
        theOS << indent(theDepth) << std::setw(W_COL1) << "else:";
        aPrintSchema(aElse);
      }
      if (const auto aAllOf = aApplicator.getAllOf()) {
        theOS << indent(theDepth) << "allOf:\n";
        for (const auto aSchema : *aAllOf)
          printSchema(theOS, aSchema, theDepth + 1, true);
      }
      if (const auto aAnyOf = aApplicator.getAnyOf()) {
        theOS << indent(theDepth) << "anyOf:\n";
        for (const auto aSchema : *aAnyOf)
          printSchema(theOS, aSchema, theDepth + 1, true);
      }
      if (const auto aOneOf = aApplicator.getOneOf()) {
        theOS << indent(theDepth) << "oneOf:\n";
        for (const auto aSchema : *aOneOf)
          printSchema(theOS, aSchema, theDepth + 1, true);
      }
      if (const auto aNot = aApplicator.getNot()) {
        theOS << indent(theDepth) << std::setw(W_COL1) << "not:";
        aPrintSchema(aNot);
      }
    };
    if (hasOutput(printTo)) {
      theOS << indent(theDepth) << "# Applicator\n";
      printTo(theOS);
    }
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

  template <typename OS>
  static void printObjectValidation(OS &theOS, const SchemaObject &theObj,
                                    const unsigned theDepth) {
    const auto &aValidation = theObj.template getSection<SchemaValidation>();
    const auto printTo = [aValidation, theDepth](auto &theOS) {
      if (const auto aMultipleOf = aValidation.getMultipleOf();
          aMultipleOf.has_value()) {
        theOS << indent(theDepth) << std::setw(W_COL1)
              << "multipleOf:" << aMultipleOf << "\n";
      }
      if (const auto aMaximum = aValidation.getMaximum();
          aMaximum.has_value()) {
        theOS << indent(theDepth) << std::setw(W_COL1) << "maximum:" << aMaximum
              << "\n";
      }
      if (const auto aExclusiveMaximum = aValidation.getExclusiveMaximum();
          aExclusiveMaximum.has_value()) {
        theOS << indent(theDepth) << std::setw(W_COL1)
              << "exclusiveMaximum:" << aExclusiveMaximum << "\n";
      }
      if (const auto aMinimum = aValidation.getMinimum();
          aMinimum.has_value()) {
        theOS << indent(theDepth) << std::setw(W_COL1) << "minimum:" << aMinimum
              << "\n";
      }
      if (const auto aExclusiveMinimum = aValidation.getExclusiveMinimum();
          aExclusiveMinimum.has_value()) {
        theOS << indent(theDepth) << std::setw(W_COL1)
              << "exclusiveMinimum:" << aExclusiveMinimum << "\n";
      }
      if (const auto aMaxLength = aValidation.getMaxLength();
          aMaxLength.has_value()) {
        theOS << indent(theDepth) << std::setw(W_COL1)
              << "maxLength:" << aMaxLength << "\n";
      }
      if (const auto aMinLength = aValidation.getMinLength();
          aMinLength.has_value()) {
        theOS << indent(theDepth) << std::setw(W_COL1)
              << "minLength:" << aMinLength << "\n";
      }
      if (const auto aMaxItems = aValidation.getMaxItems();
          aMaxItems.has_value()) {
        theOS << indent(theDepth) << std::setw(W_COL1)
              << "maxItems:" << aMaxItems << "\n";
      }
      if (const auto aMinItems = aValidation.getMinItems();
          aMinItems.has_value()) {
        theOS << indent(theDepth) << std::setw(W_COL1)
              << "minItems:" << aMinItems << "\n";
      }
      if (const auto aUniqueItems = aValidation.getUniqueItems();
          aUniqueItems.has_value()) {
        theOS << indent(theDepth) << std::setw(W_COL1)
              << "uniqueItems:" << (aUniqueItems.value() ? "true" : "false")
              << "\n";
      }
      if (const auto aMaxContains = aValidation.getMaxContains();
          aMaxContains.has_value()) {
        theOS << indent(theDepth) << std::setw(W_COL1)
              << "maxContains:" << aMaxContains << "\n";
      }
      if (const auto aMinContains = aValidation.getMinContains();
          aMinContains.has_value()) {
        theOS << indent(theDepth) << std::setw(W_COL1)
              << "minContains:" << aMinContains << "\n";
      }
      if (const auto aMaxProperties = aValidation.getMaxProperties();
          aMaxProperties.has_value()) {
        theOS << indent(theDepth) << std::setw(W_COL1)
              << "maxProperties:" << aMaxProperties << "\n";
      }
      if (const auto aMinProperties = aValidation.getMinProperties();
          aMinProperties.has_value()) {
        theOS << indent(theDepth) << std::setw(W_COL1)
              << "minProperties:" << aMinProperties << "\n";
      }
      if (const auto aPattern = aValidation.getPattern()) {
        theOS << indent(theDepth) << std::setw(W_COL1)
              << "pattern:" << *aPattern << "\n";
      }
      if (const auto aRequired = aValidation.getRequired()) {
        theOS << indent(theDepth) << "required:\n";
        for (const auto aValue : *aRequired) {
          theOS << indent(theDepth + 1) << aValue << "\n";
        }
      }
      if (const auto aDependentRequired = aValidation.getDependentRequired()) {
        theOS << indent(theDepth) << "dependentRequired:\n";
        for (const auto [aKey, aStringList] : *aDependentRequired) {
          theOS << indent(theDepth + 1) << aKey << ":\n";
          for (const auto aValue : aStringList) {
            theOS << indent(theDepth + 2) << aValue << "\n";
          }
        }
      }
      if (const auto aConst = aValidation.getConst()) {
        theOS << indent(theDepth) << "const: ";
        JsonPrinter<OS> aJsonPrinter;
        aJsonPrinter.print(theOS, *aConst) << "\n";
      }
      if (const auto aEnum = aValidation.getEnum()) {
        theOS << indent(theDepth) << "enum:\n";
        JsonPrinter<OS> aJsonPrinter;
        for (const auto aJson : *aEnum) {
          theOS << indent(theDepth + 1);
          aJsonPrinter.print(theOS, aJson) << "\n";
        }
      }
      if (const auto aTypes = aValidation.getType()) {
        theOS << indent(theDepth) << "type:\n";
        for (const Types aType : *aTypes)
          theOS << indent(theDepth + 1) << toString(aType) << "\n";
      }
    };
    if (hasOutput(printTo)) {
      theOS << indent(theDepth) << "# Validation\n";
      printTo(theOS);
    }
  }

  template <typename OS>
  static void printObjectFormat(OS &theOS, const SchemaObject &theObj,
                                const unsigned theDepth) {
    const auto &aFormat = theObj.template getSection<SchemaFormat>();
    if (const auto aFormatStr = aFormat.getFormat()) {
      theOS << indent(theDepth) << "# Format\n";
      theOS << indent(theDepth) << std::setw(W_COL1)
            << "format: " << *aFormatStr << "\n";
    }
  }

  template <typename OS>
  static void printObjectMetadata(OS &theOS, const SchemaObject &theObj,
                                  const unsigned theDepth) {
    const auto &aMetadata = theObj.template getSection<SchemaMetadata>();
    const auto printTo = [aMetadata, theDepth](auto &theOS) {
      if (const auto aTitle = aMetadata.getTitle()) {
        theOS << indent(theDepth) << std::setw(W_COL1) << "title: " << *aTitle
              << "\n";
      }
      if (const auto aDesc = aMetadata.getDescription()) {
        theOS << indent(theDepth) << std::setw(W_COL1)
              << "description: " << *aDesc << "\n";
      }
      if (const auto aDefault = aMetadata.getDefault()) {
        theOS << indent(theDepth) << "default: ";
        JsonPrinter<OS> aJsonPrinter;
        aJsonPrinter.print(theOS, *aDefault) << "\n";
      }
      if (const auto aDeprecated = aMetadata.getDeprecated();
          aDeprecated.has_value())
        theOS << indent(theDepth) << std::setw(W_COL1)
              << "deprecated: " << (aDeprecated.value() ? "true" : "false")
              << "\n";
      if (const auto aReadOnly = aMetadata.getReadOnly(); aReadOnly.has_value())
        theOS << indent(theDepth) << std::setw(W_COL1)
              << "readOnly: " << (aReadOnly.value() ? "true" : "false") << "\n";
      if (const auto aWriteOnly = aMetadata.getWriteOnly();
          aWriteOnly.has_value())
        theOS << indent(theDepth) << std::setw(W_COL1)
              << "writeOnly: " << (aWriteOnly.value() ? "true" : "false")
              << "\n";
      if (const auto aExamples = aMetadata.getExamples()) {
        theOS << indent(theDepth) << "examples:\n";
        JsonPrinter<OS> aJsonPrinter;
        for (const auto aJson : *aExamples) {
          theOS << indent(theDepth + 1);
          aJsonPrinter.print(theOS, aJson) << "\n";
        }
      }
    };
    if (hasOutput(printTo)) {
      theOS << indent(theDepth) << "# Metadata\n";
      printTo(theOS);
    }
  }

  template <typename OS>
  static void printObjectContent(OS &theOS, const SchemaObject &theObj,
                                 const unsigned theDepth) {
    const auto &aContent = theObj.template getSection<SchemaContent>();
    const auto printTo = [aContent, theDepth](auto &theOS) {
      if (const auto aContentMediaType = aContent.getContentMediaType()) {
        theOS << indent(theDepth) << std::setw(W_COL1)
              << "contentMediaType: " << *aContentMediaType << "\n";
      }
      if (const auto aContentEncoding = aContent.getContentEncoding()) {
        theOS << indent(theDepth) << std::setw(W_COL1)
              << "contentEncoding: " << *aContentEncoding << "\n";
      }
      if (const auto aContentSchema = aContent.getContentSchema()) {
        theOS << indent(theDepth) << "contentSchema:";
        printSchema(theOS, *aContentSchema, theDepth + 1);
      }
    };
    if (hasOutput(printTo)) {
      theOS << indent(theDepth) << "# Content\n";
      printTo(theOS);
    }
  }

  const SchemaObject &itsSchemaObject;
};
} // namespace json_schema
#endif // JSON_SCHEMA_SCHEMA_PRINTER_H
