#ifndef JSON_SCHEMA_2019_09_SCHEMA_STANDARD_H
#define JSON_SCHEMA_2019_09_SCHEMA_STANDARD_H
#include "json_schema/2019-09/model/applicator.h"
#include "json_schema/2019-09/model/content.h"
#include "json_schema/2019-09/model/core.h"
#include "json_schema/2019-09/model/format.h"
#include "json_schema/2019-09/model/metadata.h"
#include "json_schema/2019-09/model/validation.h"
#include "json_schema/2019-09/reader/applicator.h"
#include "json_schema/2019-09/reader/compat.h"
#include "json_schema/2019-09/reader/content.h"
#include "json_schema/2019-09/reader/core.h"
#include "json_schema/2019-09/reader/format.h"
#include "json_schema/2019-09/reader/metadata.h"
#include "json_schema/2019-09/reader/validation.h"
#include "json_schema/schema_info_reader.h"
#include "json_schema/schema_object.h"
#include "json_schema/schema_reader.h"

namespace json_schema {

template <bool LENIENT = true> struct Standard_2019_09 {
  template <typename Storage>
  using SchemaObject =
      SchemaObjectBase<Storage, SchemaCore, SchemaApplicator, SchemaMetadata,
                       SchemaValidation, SchemaFormat, SchemaContent>;

  template <typename JSON, typename ErrorHandling>
  using SchemaInfoReader = SchemaInfoReaderBase<
      LENIENT, JSON, ErrorHandling, ReaderCore::InfoReader,
      ReaderApplicator::InfoReader, ReaderMetadata::InfoReader,
      ReaderValidation::InfoReader, ReaderFormat::InfoReader,
      ReaderContent::InfoReader, ReaderCompat::InfoReader>;

  /// SchemaReader needs access to a definition of the boolean Schema
  /// definitions.
  template <typename SchemaContext> struct BoolGen {
    using SchemaRef = typename SchemaContext::SchemaRef;
    using SchemaObj = SchemaObject<typename SchemaContext::Storage>;

    template <typename SchemaAllocator>
    constexpr std::pair<SchemaRef, SchemaRef>
    makeBoolSchemas(SchemaAllocator &theSchemaAlloc,
                    SchemaContext &theContext) {
      SchemaRef aTrueRef =
          theSchemaAlloc.allocateSchema(SchemaObj{}, theContext);
      SchemaObj aFalseSchema{};
      aFalseSchema.template getSection<SchemaApplicator>().itsNot =
          SchemaContext::Storage::pointer_to(aTrueRef);
      SchemaRef aFalseRef =
          theSchemaAlloc.allocateSchema(aFalseSchema, theContext);
      return std::make_pair(aTrueRef, aFalseRef);
    }
  };

  template <typename SchemaContext, typename ErrorHandling>
  using SchemaReader =
      SchemaReaderBase<LENIENT, SchemaContext, ErrorHandling,
                       BoolGen<SchemaContext>, ReaderCore::SchemaReader,
                       ReaderApplicator::SchemaReader,
                       ReaderFormat::SchemaReader, ReaderMetadata::SchemaReader,
                       ReaderValidation::SchemaReader,
                       ReaderContent::SchemaReader, ReaderCompat::SchemaReader>;
};
} // namespace json_schema
#endif // JSON_SCHEMA_2019_09_SCHEMA_STANDARD_H
