#include "cli_args/cli_args.h"
#include "multi_lib.h"

namespace cl = ::cli_args;

const char *const TOOLNAME = "multi_tu";
const char *const TOOLDESC =
    "CLI arguments spread across multiple translation units";
static cl::opt<std::string> MainArg(cl::name("main"),
                                    cl::init("main argument initial value"));

int main(int argc, const char **argv) {
  if (!cl::ParseArgs(argc, argv))
    cl::PrintHelp(TOOLNAME, TOOLDESC, std::cout);
  std::cout << *MainArg << "\n" << getLib1Arg() << "\n" << getLib2Arg() << "\n";
}
