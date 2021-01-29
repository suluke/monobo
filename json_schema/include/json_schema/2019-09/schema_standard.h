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

struct Standard_2019_09 {
  template <typename Storage>
  using SchemaObject =
      SchemaObjectBase<Storage, SchemaCore, SchemaApplicator, SchemaMetadata,
                       SchemaValidation, SchemaFormat, SchemaContent>;
  template <typename JSON, typename ErrorHandling>
  using SchemaInfoReader =
      SchemaInfoReaderBase<JSON, ErrorHandling, ReaderApplicator::InfoReader,
                           ReaderCore::InfoReader, ReaderFormat::InfoReader,
                           ReaderMetadata::InfoReader,
                           ReaderValidation::InfoReader,
                           ReaderContent::InfoReader, ReaderCompat::InfoReader>;

  template <typename SchemaContext, typename ErrorHandling>
  using SchemaReader =
      SchemaReaderBase<SchemaContext, ErrorHandling,
                       ReaderApplicator::SchemaReader, ReaderCore::SchemaReader,
                       ReaderFormat::SchemaReader, ReaderMetadata::SchemaReader,
                       ReaderValidation::SchemaReader,
                       ReaderContent::SchemaReader, ReaderCompat::SchemaReader>;
};
} // namespace json_schema
#endif // JSON_SCHEMA_2019_09_SCHEMA_STANDARD_H
