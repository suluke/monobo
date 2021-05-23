#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "json_schema/2019-09/schema_standard.h"
#include "json_schema/2019-09/schema_validator.h"
#include "json_schema/2019-09/validate/error_detail.h"
#include "json_schema/dynamic_schema.h"

#include "constexpr_json/ext/error_is_detail.h"
#include "constexpr_json/ext/error_is_except.h"
#include "constexpr_json/ext/error_is_nullopt.h"
#include "constexpr_json/ext/stream_parser.h"

#include "cli_args/cli_args.h"
#include "cli_args/parsers/bool.h"
#include "cli_args/parsers/path.h"
#include "cli_args/parsers/string.h"

#include "json_schema_testsuite_driver_defs.h"

namespace cl = ::cli_args;
namespace fs = ::std::filesystem;

const char *const TOOLNAME = "json_schema_testsuite_driver";
const char *const TOOLDESC = "Verify JSON documents using JSON-Schema";

enum ERROR {
  OK = 0,
  ERROR_INVALID_JSON = 1,
  ERROR_MALFORMED_TEST = 2,
  ERROR_TEST_PARSING_FAILED = 3,
  ERROR_OPEN_FAILED = 11,
  ERROR_READ_FAILED = 12,
};

static cl::list<fs::path>
    gInputs(cl::meta("files"), cl::desc("File(s) to be validated"),
            cl::init({JSON_SCHEMA_TESTSUITE_BUILTIN_TESTDIRS}));

using Standard = json_schema::Standard_2019_09</*Lenient=*/true>;
using Context = json_schema::DynamicSchemaContext<Standard>;
using Reader =
    Standard::template SchemaReader<Context, cjson::ErrorWillThrow<>>;
using ReadResult = typename Reader::template ReadResult<1>;
using Schema = ReadResult::SchemaObject;
using Validator = json_schema::SchemaValidator<
    Context, cjson::ErrorWillReturnDetail<json_schema::ValidationErrorDetail>>;

static std::pair<std::string_view, std::string_view> gDisabledList[] = {
    {"additionalItems/additionalItemsasschema",
     "additionalitemsdonotmatchschema"},
    {"additionalItems/additionalItemsshouldnotlookinapplicators,invalidcase",
     "itemsdefinedinallOfarenotexamined"},
    {"additionalItems/arrayofitemswithnoadditionalItemspermitted",
     "additionalitemsarenotpermitted"},
    {"additionalItems/itemsvalidationadjuststhestartingindexforadditionalItems",
     "wrongtypeofseconditem"},
    {"additionalProperties/"
     "additionalPropertiesallowsaschemawhichshouldvalidate",
     "anadditionalinvalidpropertyisinvalid"},
    {"additionalProperties/"
     "additionalPropertiesbeingfalsedoesnotallowotherproperties",
     "anadditionalpropertyisinvalid"},
    {"additionalProperties/additionalPropertiescanexistbyitself",
     "anadditionalinvalidpropertyisinvalid"},
    {"additionalProperties/additionalPropertiesshouldnotlookinapplicators",
     "propertiesdefinedinallOfarenotexamined"},
    {"anchor/$anchorinsideanenumisnotarealidentifier",
     "inimplementationsthatstrip$anchor,thismaymatcheither$def"},
    {"anchor/$anchorinsideanenumisnotarealidentifier",
     "nomatchonenumor$refto$anchor"},
    {"content/validationofbinary-encodedmediatypedocuments",
     "aninvalidbase64stringthatisvalidJSON;validatestrue"},
    {"content/validationofbinary-encodedmediatypedocuments",
     "avalidly-encodedinvalidJSONdocument;validatestrue"},
    {"content/validationofbinary-encodedmediatypedocumentswithschema",
     "anemptyobjectasabase64-encodedJSONdocument;validatestrue"},
    {"content/validationofbinary-encodedmediatypedocumentswithschema",
     "aninvalidbase64-encodedJSONdocument;validatestrue"},
    {"content/validationofbinary-encodedmediatypedocumentswithschema",
     "aninvalidbase64stringthatisvalidJSON;validatestrue"},
    {"content/validationofbinary-encodedmediatypedocumentswithschema",
     "avalidly-encodedinvalidJSONdocument;validatestrue"},
    {"content/validationofbinarystring-encoding",
     "aninvalidbase64string(%isnotavalidcharacter);validatestrue"},
    {"content/validationofstring-encodedcontentbasedonmediatype",
     "aninvalidJSONdocument;validatestrue"},
    {"defs/validatedefinitionagainstmetaschema", "invaliddefinitionschema"},
    {"dependentRequired/dependencieswithescapedcharacters",
     "CRLFmissingdependent"},
    {"dependentRequired/dependencieswithescapedcharacters",
     "quotedquotesmissingdependent"},
    {"dependentRequired/multipledependentsrequired", "missingbothdependencies"},
    {"dependentRequired/multipledependentsrequired", "missingdependency"},
    {"dependentRequired/multipledependentsrequired", "missingotherdependency"},
    {"dependentRequired/singledependency", "missingdependency"},
    {"dependentSchemas/booleansubschemas", "objectwithbothpropertiesisinvalid"},
    {"dependentSchemas/booleansubschemas",
     "objectwithpropertyhavingschemafalseisinvalid"},
    {"dependentSchemas/dependencieswithescapedcharacters", "quotedquote"},
    {"dependentSchemas/dependencieswithescapedcharacters",
     "quotedquoteinvalidunderdependentschema"},
    {"dependentSchemas/dependencieswithescapedcharacters",
     "quotedtabinvalidunderdependentschema"},
    {"dependentSchemas/singledependency", "wrongtype"},
    {"dependentSchemas/singledependency", "wrongtypeboth"},
    {"dependentSchemas/singledependency", "wrongtypeother"},
    {"id/$idinsideanenumisnotarealidentifier", "nomatchonenumor$refto$id"},
    {"items/itemsandsubitems", "toomanyitems"},
    {"items/itemsandsubitems", "wrongitem"},
    {"minItems/minItemsvalidation", "tooshortisinvalid"},
    {"multipleOf/invalidinstanceshouldnotraiseerrorwhenfloatdivision=inf",
     "alwaysinvalid,butnaiveimplementationsmayraiseanoverflowerror"},
    {"patternProperties/multiplesimultaneouspatternPropertiesarevalidated",
     "aninvalidduetobothisinvalid"},
    {"patternProperties/multiplesimultaneouspatternPropertiesarevalidated",
     "aninvalidduetooneisinvalid"},
    {"patternProperties/multiplesimultaneouspatternPropertiesarevalidated",
     "aninvalidduetotheotherisinvalid"},
    {"patternProperties/patternPropertiesvalidatespropertiesmatchingaregex",
     "asingleinvalidmatchisinvalid"},
    {"patternProperties/patternPropertiesvalidatespropertiesmatchingaregex",
     "multipleinvalidmatchesisinvalid"},
    {"patternProperties/patternPropertieswithbooleanschemas",
     "objectwithapropertymatchingbothtrueandfalseisinvalid"},
    {"patternProperties/patternPropertieswithbooleanschemas",
     "objectwithbothpropertiesisinvalid"},
    {"patternProperties/patternPropertieswithbooleanschemas",
     "objectwithpropertymatchingschemafalseisinvalid"},
    {"patternProperties/regexesarenotanchoredbydefaultandarecasesensitive",
     "recognizedmembersareaccountedfor"},
    {"patternProperties/regexesarenotanchoredbydefaultandarecasesensitive",
     "regexesarecasesensitive,2"},
    {"properties/properties,patternProperties,additionalPropertiesinteraction",
     "additionalPropertyinvalidatesothers"},
    {"properties/properties,patternProperties,additionalPropertiesinteraction",
     "patternPropertyinvalidatesnonproperty"},
    {"properties/properties,patternProperties,additionalPropertiesinteraction",
     "patternPropertyinvalidatesproperty"},
    {"propertyNames/propertyNamesvalidation", "somepropertynamesinvalid"},
    {"propertyNames/propertyNameswithbooleanschemafalse",
     "objectwithanypropertiesisinvalid"},
    {"recursiveRef/"
     "$recursiveRefwithno$recursiveAnchorintheinitialtargetschemaresource",
     "leafnodedoesnotmatch;norecursion"},
    {"recursiveRef/$recursiveRefwithno$recursiveAnchorintheouterschemaresource",
     "leafnodedoesnotmatch;norecursion"},
    {"recursiveRef/$recursiveRefwithno$recursiveAnchorworkslike$ref",
     "integerdoesnotmatchasapropertyvalue"},
    {"recursiveRef/$recursiveRefwithno$recursiveAnchorworkslike$ref",
     "twolevels,integerdoesnotmatchasapropertyvalue"},
    {"recursiveRef/$recursiveRefwithout$recursiveAnchorworkslike$ref",
     "mismatch"},
    {"recursiveRef/$recursiveRefwithout$recursiveAnchorworkslike$ref",
     "recursivemismatch"},
    {"recursiveRef/$recursiveRefwithoutusingnesting",
     "integerdoesnotmatchasapropertyvalue"},
    {"recursiveRef/$recursiveRefwithoutusingnesting", "twolevels,nomatch"},
    {"ref/$reftobooleanschemafalse", "anyvalueisinvalid"},
    {"ref/escapedpointerref", "percentinvalid"},
    {"ref/escapedpointerref", "slashinvalid"},
    {"ref/escapedpointerref", "tildeinvalid"},
    {"ref/nestedrefs", "nestedrefinvalid"},
    {"ref/propertynamed$ref,containinganactual$ref",
     "propertynamed$refinvalid"},
    {"ref/Recursivereferencesbetweenschemas", "invalidtree"},
    {"ref/refappliesalongsidesiblingkeywords", "refinvalid"},
    {"ref/refcreatesnewscopewhenadjacenttokeywords",
     "referencedsubschemadoesn'tseeannotationsfromproperties"},
    {"ref/refswithquote", "objectwithstringsisinvalid"},
    {"ref/relativepointerreftoarray", "mismatcharray"},
    {"ref/relativepointerreftoobject", "mismatch"},
    {"ref/remoteref,containingrefsitself", "remoterefinvalid"},
    {"ref/rootpointerref", "mismatch"},
    {"ref/rootpointerref", "recursivemismatch"},
    {"refRemote/baseURIchange", "baseURIchangerefinvalid"},
    {"refRemote/fragmentwithinremoteref", "remotefragmentinvalid"},
    {"refRemote/refwithinremoteref", "refwithinrefinvalid"},
    {"refRemote/remoteref", "remoterefinvalid"},
    {"refRemote/rootrefinremoteref", "objectisinvalid"},
    {"unevaluatedItems/itemisevaluatedinanuncleschematounevaluatedItems",
     "unclekeywordevaluationisnotsignificant"},
    {"unevaluatedItems/unevaluatedItemsasschema",
     "withinvalidunevaluateditems"},
    {"unevaluatedItems/unevaluatedItemscan'tseeinsidecousins", "alwaysfails"},
    {"unevaluatedItems/unevaluatedItemsfalse", "withunevaluateditems"},
    {"unevaluatedItems/unevaluatedItemswith$ref", "withunevaluateditems"},
    {"unevaluatedItems/unevaluatedItemswithanyOf",
     "whenoneschemamatchesandhasunevaluateditems"},
    {"unevaluatedItems/unevaluatedItemswithanyOf",
     "whentwoschemasmatchandhasunevaluateditems"},
    {"unevaluatedItems/unevaluatedItemswithbooleanschemas",
     "withunevaluateditems"},
    {"unevaluatedItems/unevaluatedItemswithif/then/else",
     "whenifdoesn'tmatchandithasunevaluateditems"},
    {"unevaluatedItems/unevaluatedItemswithif/then/else",
     "whenifmatchesandithasunevaluateditems"},
    {"unevaluatedItems/unevaluatedItemswithnestedtuple",
     "withunevaluateditems"},
    {"unevaluatedItems/unevaluatedItemswithnot", "withunevaluateditems"},
    {"unevaluatedItems/unevaluatedItemswithoneOf", "withunevaluateditems"},
    {"unevaluatedItems/unevaluatedItemswithtuple", "withunevaluateditems"},
    {"unevaluatedProperties/"
     "cousinunevaluatedProperties,trueandfalse,falsewithproperties",
     "withnestedunevaluatedproperties"},
    {"unevaluatedProperties/"
     "cousinunevaluatedProperties,trueandfalse,truewithproperties",
     "withnestedunevaluatedproperties"},
    {"unevaluatedProperties/"
     "cousinunevaluatedProperties,trueandfalse,truewithproperties",
     "withnonestedunevaluatedproperties"},
    {"unevaluatedProperties/"
     "nestedunevaluatedProperties,outertrue,innerfalse,propertiesinside",
     "withnestedunevaluatedproperties"},
    {"unevaluatedProperties/"
     "nestedunevaluatedProperties,outertrue,innerfalse,propertiesoutside",
     "withnestedunevaluatedproperties"},
    {"unevaluatedProperties/"
     "nestedunevaluatedProperties,outertrue,innerfalse,propertiesoutside",
     "withnonestedunevaluatedproperties"},
    {"unevaluatedProperties/"
     "propertyisevaluatedinanuncleschematounevaluatedProperties",
     "unclekeywordevaluationisnotsignificant"},
    {"unevaluatedProperties/unevaluatedPropertiescan'tseeinsidecousins",
     "alwaysfails"},
    {"unevaluatedProperties/unevaluatedPropertiesfalse",
     "withunevaluatedproperties"},
    {"unevaluatedProperties/unevaluatedPropertiesschema",
     "withinvalidunevaluatedproperties"},
    {"unevaluatedProperties/unevaluatedPropertieswith$ref",
     "withunevaluatedproperties"},
    {"unevaluatedProperties/unevaluatedPropertieswithadjacentpatternProperties",
     "withunevaluatedproperties"},
    {"unevaluatedProperties/unevaluatedPropertieswithadjacentproperties",
     "withunevaluatedproperties"},
    {"unevaluatedProperties/unevaluatedPropertieswithanyOf",
     "whenonematchesandhasunevaluatedproperties"},
    {"unevaluatedProperties/unevaluatedPropertieswithanyOf",
     "whentwomatchandhasunevaluatedproperties"},
    {"unevaluatedProperties/unevaluatedPropertieswithbooleanschemas",
     "withunevaluatedproperties"},
    {"unevaluatedProperties/unevaluatedPropertieswithdependentSchemas",
     "withunevaluatedproperties"},
    {"unevaluatedProperties/unevaluatedPropertieswithif/then/else",
     "whenifisfalseandhasunevaluatedproperties"},
    {"unevaluatedProperties/unevaluatedPropertieswithif/then/else",
     "whenifistrueandhasunevaluatedproperties"},
    {"unevaluatedProperties/unevaluatedPropertieswithnestedpatternProperties",
     "withadditionalproperties"},
    {"unevaluatedProperties/unevaluatedPropertieswithnestedproperties",
     "withadditionalproperties"},
    {"unevaluatedProperties/unevaluatedPropertieswithnot",
     "withunevaluatedproperties"},
    {"unevaluatedProperties/unevaluatedPropertieswithoneOf",
     "withunevaluatedproperties"},
    {"uniqueItems/uniqueItems=falsewithanarrayofitemsandadditionalItems=false",
     "extraitemsareinvalidevenifunique"},
    {"uniqueItems/uniqueItemsvalidation",
     "numbersareuniqueifmathematicallyunequal"},
    {"uniqueItems/uniqueItemswithanarrayofitems",
     "[false,false]fromitemsarrayisnotvalid"},
    {"uniqueItems/uniqueItemswithanarrayofitems",
     "[true,true]fromitemsarrayisnotvalid"},
    {"uniqueItems/uniqueItemswithanarrayofitemsandadditionalItems=false",
     "[false,false]fromitemsarrayisnotvalid"},
    {"uniqueItems/uniqueItemswithanarrayofitemsandadditionalItems=false",
     "[true,true]fromitemsarrayisnotvalid"},
    {"uniqueItems/uniqueItemswithanarrayofitemsandadditionalItems=false",
     "extraitemsareinvalidevenifunique"},
};
static bool isDisabled(const std::string &theGroup, const char *const theTest) {
  static std::set<std::pair<std::string_view, std::string_view>> aDisabledSet{
      std::begin(gDisabledList), std::end(gDisabledList)};
  return aDisabledSet.count(
      std::make_pair<std::string_view, std::string_view>(theGroup, theTest));
}

class TestsuiteFixture : public testing::Test {};
class TestsuiteTest : public TestsuiteFixture {
public:
  explicit TestsuiteTest(
      const std::shared_ptr<cjson::DynamicDocument> &theJsonDoc,
      const std::shared_ptr<ReadResult> &theSchemaContext,
      const cjson::DynamicDocument::EntityRef theData,
      const bool theShouldBeValid)
      : itsJsonDoc{theJsonDoc}, itsSchemaContext{theSchemaContext},
        itsData{theData}, itsShouldBeValid{theShouldBeValid} {}
  void TestBody() override {
    Validator aValidator((*itsSchemaContext)[0]);
    const auto aErrorMaybe = aValidator.validate(itsData);
    if (itsShouldBeValid) {
      EXPECT_FALSE(aErrorMaybe) << "Schema verification finished with errors "
                                   "but no error was expected: "
                                << aErrorMaybe->what();
    } else {
      EXPECT_TRUE(aErrorMaybe) << "Schema verification finished successfully "
                                  "but was expected to fail";
    }
  }

private:
  std::shared_ptr<cjson::DynamicDocument> itsJsonDoc;
  std::shared_ptr<ReadResult> itsSchemaContext;
  cjson::DynamicDocument::EntityRef itsData;
  bool itsShouldBeValid{true};
};

struct TestReport {
  bool hasError() const noexcept { return itsHasError; }
  void addError(ERROR theEC, const char *const theMsg) { itsHasError = true; }

private:
  bool itsHasError{false};
};

struct TestGroupReport {
  bool hasError() const noexcept { return itsHasError; }
  void addError(ERROR theEC, const char *const theMsg) { itsHasError = true; }
  void addReport(const TestReport &theReport) {
    itsSingleTestReports.emplace_back(theReport);
    itsHasError |= theReport.hasError();
  }

private:
  bool itsHasError{false};
  std::vector<TestReport> itsSingleTestReports;
};

static void normalizeName4GTest(std::string &theName) {
  for (auto aIt = std::find(theName.begin(), theName.end(), ' ');
       aIt != theName.end();
       aIt = std::find(theName.begin(), theName.end(), ' ')) {
    theName.erase(aIt);
  }
}

static TestReport
processTest(const cjson::DynamicDocument::EntityRef &theTest,
            const std::string &theSuiteName,
            const std::shared_ptr<ReadResult> &theSchema,
            const std::shared_ptr<cjson::DynamicDocument> &theJson) {
  TestReport aResult;
  if (theTest.getType() != cjson::Entity::OBJECT) {
    aResult.addError(ERROR_MALFORMED_TEST,
                     "Expected test to be of type object");
    return aResult;
  }
  const auto &aTestObject = theTest.toObject();
  std::string aDesc = "<description missing>";
  if (auto aDescription = aTestObject["description"]) {
    if (aDescription->getType() == cjson::Entity::STRING)
      aDesc = aDescription->toString();
    normalizeName4GTest(aDesc);
  }
  const auto &aValid = aTestObject["valid"];
  if (!aValid || aValid->getType() != cjson::Entity::BOOL) {
    aResult.addError(ERROR_MALFORMED_TEST,
                     "`valid` key in test either not existing or not bool");
    return aResult;
  }
  const bool aShouldBeValid = aValid->toBool();
  const auto &aData = aTestObject["data"];
  if (!aData) {
    aResult.addError(ERROR_MALFORMED_TEST, "`data` not found in test");
    return aResult;
  }

  if (isDisabled(theSuiteName, aDesc.c_str()))
    aDesc = "DISABLED_" + aDesc;

  testing::RegisterTest(
      theSuiteName.c_str(), aDesc.c_str(), nullptr, nullptr, __FILE__, __LINE__,
      // Important to use the fixture type as the return type here.
      [=]() -> TestsuiteFixture * {
        return new TestsuiteTest(theJson, theSchema, *aData, aShouldBeValid);
      });

  return aResult;
}

static TestGroupReport
processTestGroup(const std::string &theSuiteName,
                 const cjson::DynamicDocument::EntityRef &theGroup,
                 const std::shared_ptr<cjson::DynamicDocument> &theJsonDoc) {
  TestGroupReport aResult;
  if (theGroup.getType() != cjson::Entity::OBJECT) {
    aResult.addError(ERROR_MALFORMED_TEST, "Expected test to be object\n");
    return aResult;
  }
  const auto &aGroupObject = theGroup.toObject();
  std::string aGroupDesc = "<group description missing>";
  if (auto aDescription = aGroupObject["description"]) {
    if (aDescription->getType() == cjson::Entity::STRING) {
      aGroupDesc = aDescription->toString();
      normalizeName4GTest(aGroupDesc);
    }
  }
  std::string aFullSuitename = theSuiteName + "/" + aGroupDesc;
  if (!aGroupObject["schema"]) {
    aResult.addError(ERROR_MALFORMED_TEST, "No schema in test\n");
    return aResult;
  }
  using ErrorHandling = Reader::ErrorHandling;
  const auto aSchemaOrError = Reader::read(*aGroupObject["schema"]);
  if (ErrorHandling::isError(aSchemaOrError)) {
    aResult.addError(ERROR_MALFORMED_TEST, "Failed to build test schema\n");
    return aResult;
  }
  const auto aSchemaReadRes =
      std::make_shared<ReadResult>(ErrorHandling::unwrap(aSchemaOrError));
  const auto &aTests = aGroupObject["tests"];
  if (aTests && aTests->getType() == cjson::Entity::ARRAY) {
    const auto &aTestsArray = aTests->toArray();
    for (const auto &aTestElm : aTestsArray)
      aResult.addReport(
          processTest(aTestElm, aFullSuitename, aSchemaReadRes, theJsonDoc));
  }
  return aResult;
}

ERROR processFilePath(const fs::path &thePath,
                      std::vector<TestGroupReport> &theReports) {
  using ErrorHandling = cjson::ErrorWillReturnDetail<cjson::JsonErrorDetail>;
  using Parser = cjson::StreamParser<ErrorHandling>;
  Parser::Result aJsonInput;
  std::string aSuiteName;
  if (thePath == "-") {
    aSuiteName = "stdin";
    aJsonInput = Parser::parse(std::cin);
  } else {
    aSuiteName = thePath.filename().stem();
    normalizeName4GTest(aSuiteName);
    std::ifstream aFileIn(thePath, std::ios::binary);
    if (!aFileIn)
      return ERROR_OPEN_FAILED;
    aJsonInput = Parser::parse(aFileIn);
  }
  if (!aJsonInput) {
    std::cerr << "Failed to read file " << thePath << "\n";
    return ERROR_READ_FAILED;
  }
  if (ErrorHandling::isError(*aJsonInput)) {
    std::cerr << "Failed to parse JSON " << thePath << "\n";
    return ERROR_INVALID_JSON;
  }
  // This converts from unique to shared_ptr to be shared between tests
  std::shared_ptr<cjson::DynamicDocument> aJsonDoc =
      std::move(ErrorHandling::unwrap(*aJsonInput));
  const auto aJsonRoot = aJsonDoc->getRoot();
  if (aJsonRoot.getType() != cjson::Entity::ARRAY) {
    std::cerr << "Expected root element of test json " << thePath
              << " to be a list\n";
    return ERROR_MALFORMED_TEST;
  }
  for (const auto &aGroup : aJsonRoot.toArray())
    theReports.emplace_back(processTestGroup(aSuiteName, aGroup, aJsonDoc));
  return OK;
}

int main(int argc, const char **argv) {
  testing::InitGoogleTest(&argc, const_cast<char **>(argv));

  if (!cl::ParseArgs(argc, argv)) {
    cl::PrintHelp(TOOLNAME, TOOLDESC, std::cout);
    return 1;
  }
  std::error_code aEc;
  std::vector<TestGroupReport> aReports;
  for (const fs::path &aPath : gInputs) {
    if (fs::is_directory(aPath)) {
      for (const auto &aDirEntry : fs::directory_iterator(aPath)) {
        if (!aDirEntry.is_directory() &&
            aDirEntry.path().extension() == ".json") {
          if (int aErrc = processFilePath(aDirEntry, aReports))
            return aErrc;
        }
      }
    } else {
      if (int aErrc = processFilePath(aPath, aReports))
        return aErrc;
    }
  }
  for (const auto &aReport : aReports) {
    if (aReport.hasError())
      return ERROR_TEST_PARSING_FAILED;
  }

  return RUN_ALL_TESTS();
}
