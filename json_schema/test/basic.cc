#include "constexpr_json/ext/error_is_except.h"
#include "constexpr_json/ext/error_is_nullopt.h"
#include "constexpr_json/static_document.h"
#include "json_schema/2019-09/schema_printer.h"
#include "json_schema/2019-09/schema_standard.h"
#include "json_schema/2019-09/schema_validator.h"
#include "json_schema/dynamic_schema.h"
#include "json_schema/static_schema.h"

#include <iostream>

using namespace json_schema;

std::ostream &operator<<(std::ostream &theOS, const SchemaInfo &aSchemaInfo) {
  return theOS << "Number of Schema Objects:             "
               << aSchemaInfo.NUM_SCHEMA_OBJECTS << "\n"
               << "Number of chars in strings:           "
               << aSchemaInfo.NUM_CHARS << "\n"
               << "Number of vocabulary entries:         "
               << aSchemaInfo.NUM_VOCAB_ENTRIES << "\n"
               << "Number of string->schema map entries: "
               << aSchemaInfo.NUM_SCHEMA_DICT_ENTRIES << "\n"
               << "Number of schema list items:          "
               << aSchemaInfo.NUM_SCHEMA_LIST_ITEMS << "\n";
}

template <typename ContextTy, typename... JSONs>
constexpr auto readSchemaIgnoreError(JSONs &&...theJsons) {
  return ContextTy::Standard::template SchemaReader<
      ContextTy, cjson::ErrorWillThrow>::read(std::forward<JSONs>(theJsons)...);
}

int main() {
  using Standard = Standard_2019_09<>;
  // using ErrorHandling = cjson::ErrorWillReturnNone;
  // using ErrorHandling = cjson::ErrorWillThrow;

/*
 * The following schema inclusion blocks can simply be copy-pasted
 * - only NAME and included header need to be adapted (+ title comment)
 */
// === Core ===
#define NAME Core
#define JSON_SCHEMA_STD Standard
#define HEADER "meta/core.json.h"
#include "schema_loading_helper.h"

// === Applicator ===
#define NAME Applicator
#define JSON_SCHEMA_STD Standard
#define HEADER "meta/applicator.json.h"
#include "schema_loading_helper.h"

// === Validation ===
#define NAME Validation
#define JSON_SCHEMA_STD Standard
#define HEADER "meta/validation.json.h"
#include "schema_loading_helper.h"

// === Meta-Data ===
#define NAME Metadata
#define JSON_SCHEMA_STD Standard
#define HEADER "meta/meta-data.json.h"
#include "schema_loading_helper.h"

// === Format ===
#define NAME Format
#define JSON_SCHEMA_STD Standard
#define HEADER "meta/format.json.h"
#include "schema_loading_helper.h"

// === Content ===
#define NAME Content
#define JSON_SCHEMA_STD Standard
#define HEADER "meta/content.json.h"
#include "schema_loading_helper.h"

// === Root ===
#define NAME Root
#define JSON_SCHEMA_STD Standard
#define HEADER "schema.json.h"
#include "schema_loading_helper.h"

  { // Static
    constexpr auto aSchemaInfo = *aRootInfo + *aCoreInfo + *aApplicatorInfo +
                                 *aValidationInfo + *aMetadataInfo +
                                 *aFormatInfo + *aContentInfo;
    std::cout << aSchemaInfo << "\n";
    using ContextTy = JSON_SCHEMA_STATIC_CONTEXT_TYPE(Standard, aSchemaInfo);

    // Finally start doing some stuff on the read schema
    constexpr auto aReadResult = readSchemaIgnoreError<ContextTy>(
        aRootDoc->getRoot(), aCoreDoc->getRoot(), aApplicatorDoc->getRoot(),
        aValidationDoc->getRoot(), aMetadataDoc->getRoot(),
        aFormatDoc->getRoot(), aContentDoc->getRoot());
    using SchemaObject = std::decay_t<decltype(aReadResult)>::SchemaObject;
    static_assert(aReadResult.itsSchemas.size() == 7u,
                  "Number of schemas read does not "
                  "match number of input schemas");
    static_assert(*std::get<const SchemaObject>(aReadResult[0])
                       .getSection<SchemaCore>()
                       .getId()
                       .toString() ==
                  "https://json-schema.org/draft/2019-09/schema");

    constexpr SchemaValidator aSchemaValidator(aReadResult.itsSchemas[0],
                                               aReadResult.itsContext);
    constexpr auto aValidateRes = aSchemaValidator.validate(aValidationJson);
    std::cout << SchemaPrinter(std::get<const SchemaObject>(aReadResult[0]))
              << "(" << (aValidateRes ? "invalid" : "valid") << ")"
              << "\n";
  }
  // { // Dynamic
  //   auto aReadResult = readSchemaIgnoreError<DynamicSchemaContext<Standard>>(
  //       aRootDoc->getRoot(), aCoreDoc->getRoot(), aApplicatorDoc->getRoot(),
  //       aValidationDoc->getRoot(), aMetadataDoc->getRoot(),
  //       aFormatDoc->getRoot(), aContentDoc->getRoot());
  //   using SchemaObject = std::decay_t<decltype(aReadResult)>::SchemaObject;
  //   std::cout << SchemaPrinter(std::get<const SchemaObject>(aReadResult[0]))
  //             << "\n";
  // }
  return 0;
}
