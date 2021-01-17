#include "constexpr_json/ext/error_is_except.h"
#include "constexpr_json/ext/error_is_nullopt.h"
#include "constexpr_json/ext/stream_parser.h"
#include "constexpr_json/static_document.h"
#include "json_schema/reader/schema_info_reader.h"
#include "json_schema/schema_printer.h"
#include "json_schema/schema_reader.h"
#include "json_schema/schema_validator.h"
#include "json_schema/static_schema.h"

#include <iostream>

using namespace json_schema;

std::ostream &operator<<(std::ostream &theOS, const SchemaInfo &aSchemaInfo) {
  return theOS << "Number of Schemas:                    "
               << aSchemaInfo.NUM_SCHEMAS << "\n"
               << "Number of chars in strings:           "
               << aSchemaInfo.NUM_CHARS << "\n"
               << "Number of vocabulary entries:         "
               << aSchemaInfo.NUM_VOCAB_ENTRIES << "\n"
               << "Number of string->schema map entries: "
               << aSchemaInfo.NUM_SCHEMA_DICT_ENTRIES << "\n"
               << "Number of schema buffer items:        "
               << aSchemaInfo.NUM_SCHEMA_BUFFER_ITEMS << "\n";
}

int main() {
/*
 * The following schema inclusion blocks can simply be copy-pasted
 * - only NAME and included header need to be adapted (+ title comment)
 */
// === Core ===
#define NAME Core
#define HEADER "meta/core.json.h"
#include "schema_loading_helper.h"

// === Applicator ===
#define NAME Applicator
#define HEADER "meta/applicator.json.h"
#include "schema_loading_helper.h"

// === Validation ===
#define NAME Validation
#define HEADER "meta/validation.json.h"
#include "schema_loading_helper.h"

// === Meta-Data ===
#define NAME Metadata
#define HEADER "meta/meta-data.json.h"
#include "schema_loading_helper.h"

// === Format ===
#define NAME Format
#define HEADER "meta/format.json.h"
#include "schema_loading_helper.h"

// === Content ===
#define NAME Content
#define HEADER "meta/content.json.h"
#include "schema_loading_helper.h"

// === Root ===
#define NAME Root
#define HEADER "schema.json.h"
#include "schema_loading_helper.h"

  constexpr auto aSchemaInfo = *aRootInfo + *aCoreInfo + *aApplicatorInfo +
                               *aValidationInfo + *aMetadataInfo +
                               *aFormatInfo + *aContentInfo;
  std::cout << aSchemaInfo;
  using ContextTy = JSON_SCHEMA_STATIC_CONTEXT_TYPE(aSchemaInfo);
  using ErrorHandling = cjson::ErrorWillReturnNone;
  // using ErrorHandling = cjson::ErrorWillThrow;

  constexpr auto aReadResultOrError =
      SchemaReader<ContextTy, ErrorHandling>::read(
          aRootDoc->getRoot(), aCoreDoc->getRoot(), aApplicatorDoc->getRoot(),
          aValidationDoc->getRoot(), aMetadataDoc->getRoot(),
          aFormatDoc->getRoot(), aContentDoc->getRoot());
  static_assert(!ErrorHandling::isError(aReadResultOrError),
                "Failed to read schemas");
  constexpr auto aReadResult = ErrorHandling::unwrap(aReadResultOrError);

  constexpr SchemaValidator aSchemaValidator(aReadResult.itsSchemas[0],
                                             aReadResult.itsContext);
  constexpr auto aValidateRes = aSchemaValidator.validate(aValidationJson);
  std::cout << aSchemaInfo
            << SchemaPrinter(aReadResult.itsSchemas[0], aReadResult.itsContext)
            << "(" << (aValidateRes ? "invalid" : "valid") << ")"
            << "\n";
  return 0;
}
