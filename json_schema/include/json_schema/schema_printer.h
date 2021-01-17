#ifndef JSON_SCHEMA_SCHEMA_PRINTER_H
#define JSON_SCHEMA_SCHEMA_PRINTER_H

#include <iostream>

#include "json_schema/model/core/core.h"

namespace json_schema {

struct SchemaPrinterBase {
  friend std::ostream &operator<<(std::ostream &theOS,
                                  const SchemaPrinterBase &thePrinter) {
    thePrinter.print(theOS);
    return theOS;
  }
  virtual void print(std::ostream &theOS) const = 0;
};

template <typename Schema, typename Context>
class SchemaPrinter : public SchemaPrinterBase {
public:
  SchemaPrinter(const Schema &theSchema, const Context &theContext)
      : itsSchema(theSchema), itsContext(theContext) {}
  void print(std::ostream &theOS) const override {
    const auto &aCore = itsContext.getSchemaObject(itsSchema).template getSection<SchemaCore>();
    theOS << itsContext.getString(aCore.itsId.toString()) << "\n";
  }

private:
  const Schema &itsSchema;
  const Context &itsContext;
};
} // namespace json_schema
#endif // JSON_SCHEMA_SCHEMA_PRINTER_H
