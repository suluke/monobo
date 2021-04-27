#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "json_schema/2019-09/schema_standard.h"
#include "json_schema/2019-09/schema_validator.h"
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

using Standard = json_schema::Standard_2019_09</*Lenient=*/false>;
using Context = json_schema::DynamicSchemaContext<Standard>;
using Reader = Standard::template SchemaReader<Context, cjson::ErrorWillThrow>;
using ReadResult = typename Reader::template ReadResult<1>;
using Schema = ReadResult::SchemaObject;
using Validator = json_schema::SchemaValidator<Context>;

static std::pair<std::string_view, std::string_view> gDisabledList[] = {
    {"$anchorinsideanenumisnotarealidentifier",
     "inimplementationsthatstrip$anchor,thismaymatcheither$def"},
    {"$anchorinsideanenumisnotarealidentifier", "nomatchonenumor$refto$anchor"},
    {"$idinsideanenumisnotarealidentifier", "nomatchonenumor$refto$id"},
    {"$recursiveRefwithno$recursiveAnchorintheinitialtargetschemaresource",
     "leafnodedoesnotmatch;norecursion"},
    {"$recursiveRefwithno$recursiveAnchorintheouterschemaresource",
     "leafnodedoesnotmatch;norecursion"},
    {"$recursiveRefwithno$recursiveAnchorworkslike$ref",
     "integerdoesnotmatchasapropertyvalue"},
    {"$recursiveRefwithno$recursiveAnchorworkslike$ref",
     "twolevels,integerdoesnotmatchasapropertyvalue"},
    {"$recursiveRefwithout$recursiveAnchorworkslike$ref", "mismatch"},
    {"$recursiveRefwithout$recursiveAnchorworkslike$ref", "recursivemismatch"},
    {"$recursiveRefwithoutusingnesting", "integerdoesnotmatchasapropertyvalue"},
    {"$recursiveRefwithoutusingnesting", "twolevels,nomatch"},
    {"$reftobooleanschemafalse", "anyvalueisinvalid"},
    {"additionalItemsasschema", "additionalitemsdonotmatchschema"},
    {"additionalItemsshouldnotlookinapplicators,invalidcase",
     "itemsdefinedinallOfarenotexamined"},
    {"additionalPropertiesallowsaschemawhichshouldvalidate",
     "anadditionalinvalidpropertyisinvalid"},
    {"additionalPropertiesbeingfalsedoesnotallowotherproperties",
     "anadditionalpropertyisinvalid"},
    {"additionalPropertiescanexistbyitself",
     "anadditionalinvalidpropertyisinvalid"},
    {"additionalPropertiesshouldnotlookinapplicators",
     "propertiesdefinedinallOfarenotexamined"},
    {"allOf", "mismatchfirst"},
    {"allOf", "mismatchsecond"},
    {"allOf", "wrongtype"},
    {"allOfsimpletypes", "mismatchone"},
    {"allOfwithbaseschema", "mismatchbaseschema"},
    {"allOfwithbaseschema", "mismatchboth"},
    {"allOfwithbaseschema", "mismatchfirstallOf"},
    {"allOfwithbaseschema", "mismatchsecondallOf"},
    {"allOfwithbooleanschemas,allfalse", "anyvalueisinvalid"},
    {"allOfwithbooleanschemas,somefalse", "anyvalueisinvalid"},
    {"allOfwiththefirstemptyschema", "stringisinvalid"},
    {"allOfwiththelastemptyschema", "stringisinvalid"},
    {"anarrayofschemasforitems", "wrongtypes"},
    {"anyOf", "neitheranyOfvalid"},
    {"anyOfcomplextypes", "neitheranyOfvalid(complex)"},
    {"anyOfwithbaseschema", "bothanyOfinvalid"},
    {"anyOfwithbaseschema", "mismatchbaseschema"},
    {"anyOfwithbooleanschemas,allfalse", "anyvalueisinvalid"},
    {"arrayofitemswithnoadditionalItemspermitted",
     "additionalitemsarenotpermitted"},
    {"aschemagivenforitems", "wrongtypeofitems"},
    {"baseURIchange", "baseURIchangerefinvalid"},
    {"booleanschema'false'", "arrayisinvalid"},
    {"booleanschema'false'", "booleanfalseisinvalid"},
    {"booleanschema'false'", "booleantrueisinvalid"},
    {"booleanschema'false'", "emptyarrayisinvalid"},
    {"booleanschema'false'", "emptyobjectisinvalid"},
    {"booleanschema'false'", "nullisinvalid"},
    {"booleanschema'false'", "numberisinvalid"},
    {"booleanschema'false'", "objectisinvalid"},
    {"booleanschema'false'", "stringisinvalid"},
    {"booleansubschemas", "objectwithbothpropertiesisinvalid"},
    {"booleansubschemas", "objectwithpropertyhavingschemafalseisinvalid"},
    {"byint", "intbyintfail"},
    {"bynumber", "35isnotmultipleof1.5"},
    {"bysmallnumber", "0.00751isnotmultipleof0.0001"},
    {"constvalidation", "anothertypeisinvalid"},
    {"constvalidation", "anothervalueisinvalid"},
    {"constwith[false]doesnotmatch[0]", "[0.0]isinvalid"},
    {"constwith[false]doesnotmatch[0]", "[0]isinvalid"},
    {"constwith[true]doesnotmatch[1]", "[1.0]isinvalid"},
    {"constwith[true]doesnotmatch[1]", "[1]isinvalid"},
    {"constwith1doesnotmatchtrue", "trueisinvalid"},
    {"constwitharray", "anotherarrayitemisinvalid"},
    {"constwitharray", "arraywithadditionalitemsisinvalid"},
    {"constwithfalsedoesnotmatch0", "floatzeroisinvalid"},
    {"constwithfalsedoesnotmatch0", "integerzeroisinvalid"},
    {"constwithnull", "notnullisinvalid"},
    {"constwithobject", "anotherobjectisinvalid"},
    {"constwithobject", "anothertypeisinvalid"},
    {"constwithtruedoesnotmatch1", "floatoneisinvalid"},
    {"constwithtruedoesnotmatch1", "integeroneisinvalid"},
    {"containskeywordvalidation", "arraywithoutitemsmatchingschemaisinvalid"},
    {"containskeywordvalidation", "emptyarrayisinvalid"},
    {"containskeywordwithbooleanschemafalse", "emptyarrayisinvalid"},
    {"containskeywordwithbooleanschematrue", "emptyarrayisinvalid"},
    {"containskeywordwithconstkeyword", "arraywithoutitem5isinvalid"},
    {"cousinunevaluatedProperties,trueandfalse,falsewithproperties",
     "withnestedunevaluatedproperties"},
    {"cousinunevaluatedProperties,trueandfalse,truewithproperties",
     "withnestedunevaluatedproperties"},
    {"cousinunevaluatedProperties,trueandfalse,truewithproperties",
     "withnonestedunevaluatedproperties"},
    {"dependencieswithescapedcharacters", "CRLFmissingdependent"},
    {"dependencieswithescapedcharacters", "quotedquote"},
    {"dependencieswithescapedcharacters",
     "quotedquoteinvalidunderdependentschema"},
    {"dependencieswithescapedcharacters", "quotedquotesmissingdependent"},
    {"dependencieswithescapedcharacters",
     "quotedtabinvalidunderdependentschema"},
    {"enumsinproperties", "missingallpropertiesisinvalid"},
    {"enumsinproperties", "missingrequiredpropertyisinvalid"},
    {"enumsinproperties", "wrongbarvalue"},
    {"enumsinproperties", "wrongfoovalue"},
    {"enumwith0doesnotmatchfalse", "falseisinvalid"},
    {"enumwith1doesnotmatchtrue", "trueisinvalid"},
    {"enumwithescapedcharacters", "anotherstringisinvalid"},
    {"enumwithfalsedoesnotmatch0", "floatzeroisinvalid"},
    {"enumwithfalsedoesnotmatch0", "integerzeroisinvalid"},
    {"enumwithtruedoesnotmatch1", "floatoneisinvalid"},
    {"enumwithtruedoesnotmatch1", "integeroneisinvalid"},
    {"escapedpointerref", "percentinvalid"},
    {"escapedpointerref", "slashinvalid"},
    {"escapedpointerref", "tildeinvalid"},
    {"evaluatingthesameschemalocationagainstthesamedatalocationtwiceisnotasigno"
     "fa"
     "ninfiniteloop",
     "failingcase"},
    {"exclusiveMaximumvalidation", "abovetheexclusiveMaximumisinvalid"},
    {"exclusiveMaximumvalidation", "boundarypointisinvalid"},
    {"exclusiveMinimumvalidation", "belowtheexclusiveMinimumisinvalid"},
    {"exclusiveMinimumvalidation", "boundarypointisinvalid"},
    {"forbiddenproperty", "propertypresent"},
    {"fragmentwithinremoteref", "remotefragmentinvalid"},
    {"heterogeneousenumvalidation", "extrapropertiesinobjectisinvalid"},
    {"heterogeneousenumvalidation", "objectsaredeepcompared"},
    {"heterogeneousenumvalidation", "somethingelseisinvalid"},
    {"ifandelsewithoutthen", "invalidthroughelse"},
    {"ifandthenwithoutelse", "invalidthroughthen"},
    {"ifappearsattheendwhenserialized(keywordprocessingsequence)",
     "invalidredirectstoelseandfails"},
    {"ifappearsattheendwhenserialized(keywordprocessingsequence)",
     "noredirectstothenandfails"},
    {"ifwithbooleanschemafalse",
     "booleanschemafalseinifalwayschoosestheelsepath(invalid)"},
    {"ifwithbooleanschematrue",
     "booleanschematrueinifalwayschoosesthethenpath(invalid)"},
    {"invalidinstanceshouldnotraiseerrorwhenfloatdivision=inf",
     "alwaysinvalid,butnaiveimplementationsmayraiseanoverflowerror"},
    {"itemisevaluatedinanuncleschematounevaluatedItems",
     "unclekeywordevaluationisnotsignificant"},
    {"items+contains", "doesnotmatchitems,matchescontains"},
    {"items+contains", "matchesitems,doesnotmatchcontains"},
    {"items+contains", "matchesneitheritemsnorcontains"},
    {"itemsandsubitems", "toomanyitems"},
    {"itemsandsubitems", "wrongitem"},
    {"itemsvalidationadjuststhestartingindexforadditionalItems",
     "wrongtypeofseconditem"},
    {"itemswithbooleanschemas", "arraywithtwoitemsisinvalid"},
    {"maxContains<minContains", "emptydata"},
    {"maxContains<minContains", "invalidmaxContains"},
    {"maxContains<minContains", "invalidmaxContainsandminContains"},
    {"maxContains<minContains", "invalidminContains"},
    {"maxContains=minContains", "allelementsmatch,invalidmaxContains"},
    {"maxContains=minContains", "allelementsmatch,invalidminContains"},
    {"maxContains=minContains", "emptydata"},
    {"maxContainswithcontains", "allelementsmatch,invalidmaxContains"},
    {"maxContainswithcontains", "emptydata"},
    {"maxContainswithcontains", "someelementsmatch,invalidmaxContains"},
    {"maximumvalidation", "abovethemaximumisinvalid"},
    {"maximumvalidationwithunsignedinteger", "abovethemaximumisinvalid"},
    {"maxItemsvalidation", "toolongisinvalid"},
    {"maxLengthvalidation", "toolongisinvalid"},
    {"minContains<maxContains", "actual<minContains<maxContains"},
    {"minContains<maxContains", "minContains<maxContains<actual"},
    {"minContains=1withcontains", "emptydata"},
    {"minContains=1withcontains", "noelementsmatch"},
    {"minContains=2withcontains", "allelementsmatch,invalidminContains"},
    {"minContains=2withcontains", "emptydata"},
    {"minContains=2withcontains", "someelementsmatch,invalidminContains"},
    {"minimumvalidation", "belowtheminimumisinvalid"},
    {"minimumvalidationwithsignedinteger", "floatbelowtheminimumisinvalid"},
    {"minimumvalidationwithsignedinteger", "intbelowtheminimumisinvalid"},
    {"minItemsvalidation", "tooshortisinvalid"},
    {"minLengthvalidation", "onesupplementaryUnicodecodepointisnotlongenough"},
    {"minLengthvalidation", "tooshortisinvalid"},
    {"multipledependentsrequired", "missingbothdependencies"},
    {"multipledependentsrequired", "missingdependency"},
    {"multipledependentsrequired", "missingotherdependency"},
    {"multiplesimultaneouspatternPropertiesarevalidated",
     "aninvalidduetobothisinvalid"},
    {"multiplesimultaneouspatternPropertiesarevalidated",
     "aninvalidduetooneisinvalid"},
    {"multiplesimultaneouspatternPropertiesarevalidated",
     "aninvalidduetotheotherisinvalid"},
    {"naivereplacementof$refwithitsdestinationisnotcorrect",
     "donotevaluatethe$refinsidetheenum,definitionexactmatch"},
    {"naivereplacementof$refwithitsdestinationisnotcorrect",
     "donotevaluatethe$refinsidetheenum,matchinganystring"},
    {"nesteditems", "nestedarraywithinvalidtype"},
    {"nesteditems", "notdeepenough"},
    {"nestedrefs", "nestedrefinvalid"},
    {"nestedunevaluatedProperties,outertrue,innerfalse,propertiesinside",
     "withnestedunevaluatedproperties"},
    {"nestedunevaluatedProperties,outertrue,innerfalse,propertiesoutside",
     "withnestedunevaluatedproperties"},
    {"nestedunevaluatedProperties,outertrue,innerfalse,propertiesoutside",
     "withnonestedunevaluatedproperties"},
    {"not", "disallowed"},
    {"notmorecomplexschema", "mismatch"},
    {"notmultipletypes", "mismatch"},
    {"notmultipletypes", "othermismatch"},
    {"notwithbooleanschematrue", "anyvalueisinvalid"},
    {"nulcharactersinstrings", "donotmatchstringlackingnul"},
    {"nulcharactersinstrings", "donotmatchstringlackingnul"},
    {"objectpropertiesvalidation", "bothpropertiesinvalidisinvalid"},
    {"objectpropertiesvalidation", "onepropertyinvalidisinvalid"},
    {"oneOf", "bothoneOfvalid"},
    {"oneOf", "neitheroneOfvalid"},
    {"oneOfcomplextypes", "bothoneOfvalid(complex)"},
    {"oneOfcomplextypes", "neitheroneOfvalid(complex)"},
    {"oneOfwithbaseschema", "bothoneOfvalid"},
    {"oneOfwithbaseschema", "mismatchbaseschema"},
    {"oneOfwithbooleanschemas,allfalse", "anyvalueisinvalid"},
    {"oneOfwithbooleanschemas,alltrue", "anyvalueisinvalid"},
    {"oneOfwithbooleanschemas,morethanonetrue", "anyvalueisinvalid"},
    {"oneOfwithmissingoptionalproperty", "bothoneOfvalid"},
    {"oneOfwithmissingoptionalproperty", "neitheroneOfvalid"},
    {"patternPropertiesvalidatespropertiesmatchingaregex",
     "asingleinvalidmatchisinvalid"},
    {"patternPropertiesvalidatespropertiesmatchingaregex",
     "multipleinvalidmatchesisinvalid"},
    {"patternPropertieswithbooleanschemas",
     "objectwithapropertymatchingbothtrueandfalseisinvalid"},
    {"patternPropertieswithbooleanschemas",
     "objectwithbothpropertiesisinvalid"},
    {"patternPropertieswithbooleanschemas",
     "objectwithpropertymatchingschemafalseisinvalid"},
    {"properties,patternProperties,additionalPropertiesinteraction",
     "additionalPropertyinvalidatesothers"},
    {"properties,patternProperties,additionalPropertiesinteraction",
     "patternPropertyinvalidatesnonproperty"},
    {"properties,patternProperties,additionalPropertiesinteraction",
     "patternPropertyinvalidatesproperty"},
    {"properties,patternProperties,additionalPropertiesinteraction",
     "propertyinvalidatesproperty"},
    {"propertieswithbooleanschema", "bothpropertiespresentisinvalid"},
    {"propertieswithbooleanschema", "only'false'propertypresentisinvalid"},
    {"propertieswithescapedcharacters", "objectwithstringsisinvalid"},
    {"propertyisevaluatedinanuncleschematounevaluatedProperties",
     "unclekeywordevaluationisnotsignificant"},
    {"propertynamed$ref,containinganactual$ref", "propertynamed$refinvalid"},
    {"propertynamed$refthatisnotareference", "propertynamed$refinvalid"},
    {"propertyNamesvalidation", "somepropertynamesinvalid"},
    {"propertyNameswithbooleanschemafalse", "objectwithanypropertiesisinvalid"},
    {"Recursivereferencesbetweenschemas", "invalidtree"},
    {"refappliesalongsidesiblingkeywords", "refinvalid"},
    {"refappliesalongsidesiblingkeywords", "refvalid,maxItemsinvalid"},
    {"refcreatesnewscopewhenadjacenttokeywords",
     "referencedsubschemadoesn'tseeannotationsfromproperties"},
    {"refswithquote", "objectwithstringsisinvalid"},
    {"refwithinremoteref", "refwithinrefinvalid"},
    {"regexesarenotanchoredbydefaultandarecasesensitive",
     "recognizedmembersareaccountedfor"},
    {"regexesarenotanchoredbydefaultandarecasesensitive",
     "regexesarecasesensitive,2"},
    {"relativepointerreftoarray", "mismatcharray"},
    {"relativepointerreftoobject", "mismatch"},
    {"remoteref,containingrefsitself", "remoterefinvalid"},
    {"remoteref", "remoterefinvalid"},
    {"requiredwithescapedcharacters",
     "objectwithsomepropertiesmissingisinvalid"},
    {"rootpointerref", "mismatch"},
    {"rootpointerref", "recursivemismatch"},
    {"rootrefinremoteref", "objectisinvalid"},
    {"simpleenumvalidation", "somethingelseisinvalid"},
    {"singledependency", "missingdependency"},
    {"singledependency", "wrongtype"},
    {"singledependency", "wrongtypeboth"},
    {"singledependency", "wrongtypeother"},
    {"thedefaultkeyworddoesnotdoanythingifthepropertyismissing",
     "anexplicitpropertyvalueischeckedagainstmaximum(failing)"},
    {"typeasarraywithoneitem", "numberisinvalid"},
    {"unevaluatedItemsasschema", "withinvalidunevaluateditems"},
    {"unevaluatedItemscan'tseeinsidecousins", "alwaysfails"},
    {"unevaluatedItemsfalse", "withunevaluateditems"},
    {"unevaluatedItemswith$ref", "withunevaluateditems"},
    {"unevaluatedItemswithanyOf", "whenoneschemamatchesandhasunevaluateditems"},
    {"unevaluatedItemswithanyOf", "whentwoschemasmatchandhasunevaluateditems"},
    {"unevaluatedItemswithbooleanschemas", "withunevaluateditems"},
    {"unevaluatedItemswithif/then/else",
     "whenifdoesn'tmatchandithasunevaluateditems"},
    {"unevaluatedItemswithif/then/else",
     "whenifmatchesandithasunevaluateditems"},
    {"unevaluatedItemswithnestedtuple", "withunevaluateditems"},
    {"unevaluatedItemswithnot", "withunevaluateditems"},
    {"unevaluatedItemswithoneOf", "withunevaluateditems"},
    {"unevaluatedItemswithtuple", "withunevaluateditems"},
    {"unevaluatedPropertiescan'tseeinsidecousins", "alwaysfails"},
    {"unevaluatedPropertiesfalse", "withunevaluatedproperties"},
    {"unevaluatedPropertiesschema", "withinvalidunevaluatedproperties"},
    {"unevaluatedPropertieswith$ref", "withunevaluatedproperties"},
    {"unevaluatedPropertieswithadjacentpatternProperties",
     "withunevaluatedproperties"},
    {"unevaluatedPropertieswithadjacentproperties",
     "withunevaluatedproperties"},
    {"unevaluatedPropertieswithanyOf",
     "whenonematchesandhasunevaluatedproperties"},
    {"unevaluatedPropertieswithanyOf",
     "whentwomatchandhasunevaluatedproperties"},
    {"unevaluatedPropertieswithbooleanschemas", "withunevaluatedproperties"},
    {"unevaluatedPropertieswithdependentSchemas", "withunevaluatedproperties"},
    {"unevaluatedPropertieswithif/then/else",
     "whenifisfalseandhasunevaluatedproperties"},
    {"unevaluatedPropertieswithif/then/else",
     "whenifistrueandhasunevaluatedproperties"},
    {"unevaluatedPropertieswithnestedpatternProperties",
     "withadditionalproperties"},
    {"unevaluatedPropertieswithnestedproperties", "withadditionalproperties"},
    {"unevaluatedPropertieswithnot", "withunevaluatedproperties"},
    {"unevaluatedPropertieswithoneOf", "withunevaluatedproperties"},
    {"uniqueItems=falsewithanarrayofitemsandadditionalItems=false",
     "extraitemsareinvalidevenifunique"},
    {"uniqueItemsvalidation", "numbersareuniqueifmathematicallyunequal"},
    {"uniqueItemswithanarrayofitems", "[false,false]fromitemsarrayisnotvalid"},
    {"uniqueItemswithanarrayofitems", "[true,true]fromitemsarrayisnotvalid"},
    {"uniqueItemswithanarrayofitemsandadditionalItems=false",
     "[false,false]fromitemsarrayisnotvalid"},
    {"uniqueItemswithanarrayofitemsandadditionalItems=false",
     "[true,true]fromitemsarrayisnotvalid"},
    {"uniqueItemswithanarrayofitemsandadditionalItems=false",
     "extraitemsareinvalidevenifunique"},
    {"validateagainstcorrectbranch,thenvselse", "invalidthroughelse"},
    {"validateagainstcorrectbranch,thenvselse", "invalidthroughthen"},
    {"validatedefinitionagainstmetaschema", "invaliddefinitionschema"},
    {"validationofbinary-encodedmediatypedocuments",
     "aninvalidbase64stringthatisvalidJSON;validatestrue"},
    {"validationofbinary-encodedmediatypedocuments",
     "avalidly-encodedinvalidJSONdocument;validatestrue"},
    {"validationofbinary-encodedmediatypedocumentswithschema",
     "anemptyobjectasabase64-encodedJSONdocument;validatestrue"},
    {"validationofbinary-encodedmediatypedocumentswithschema",
     "aninvalidbase64-encodedJSONdocument;validatestrue"},
    {"validationofbinary-encodedmediatypedocumentswithschema",
     "aninvalidbase64stringthatisvalidJSON;validatestrue"},
    {"validationofbinary-encodedmediatypedocumentswithschema",
     "avalidly-encodedinvalidJSONdocument;validatestrue"},
    {"validationofbinarystring-encoding",
     "aninvalidbase64string(%isnotavalidcharacter);validatestrue"},
    {"validationofstring-encodedcontentbasedonmediatype",
     "aninvalidJSONdocument;validatestrue"}};
static bool isDisabled(const char *const theGroup, const char *const theTest) {
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
      EXPECT_FALSE(aErrorMaybe);
    } else {
      EXPECT_TRUE(aErrorMaybe);
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
            const char *const theGroupName,
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

  if (isDisabled(theGroupName, aDesc.c_str()))
    return aResult;

  testing::RegisterTest(
      theGroupName, aDesc.c_str(), nullptr, nullptr, __FILE__, __LINE__,
      // Important to use the fixture type as the return type here.
      [=]() -> TestsuiteFixture * {
        return new TestsuiteTest(theJson, theSchema, *aData, aShouldBeValid);
      });

  return aResult;
}

static TestGroupReport
processTestGroup(const cjson::DynamicDocument::EntityRef &theGroup,
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
      aResult.addReport(processTest(aTestElm, aGroupDesc.c_str(),
                                    aSchemaReadRes, theJsonDoc));
  }
  return aResult;
}

ERROR processFilePath(const fs::path &thePath,
                      std::vector<TestGroupReport> &theReports) {
  using ErrorHandling = cjson::ErrorWillReturnDetail<cjson::JsonErrorDetail>;
  using Parser = cjson::StreamParser<ErrorHandling>;
  Parser::Result aJsonInput;
  if (thePath == "-") {
    aJsonInput = Parser::parse(std::cin);
  } else {
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
    theReports.emplace_back(processTestGroup(aGroup, aJsonDoc));
  return OK;
}

int main(int argc, const char **argv) {
  cl::cfg::onunrecognized() =
      [&](const std::string_view &theName,
          const cl::cfg::string_span &theValues) -> int {
    if (theName.substr(0, 6) == "gtest_")
      return static_cast<int>(theValues.size());
    return -1;
  };
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

  testing::InitGoogleTest(&argc, const_cast<char**>(argv));
  return RUN_ALL_TESTS();
}
