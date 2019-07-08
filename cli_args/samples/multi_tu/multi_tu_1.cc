#include "cli_args/cli_args.h"
#include "multi_tu.h"

namespace cl = ::cli_args;

const char *const TOOLNAME = "multi_tu";
const char *const TOOLDESC =
    "CLI arguments spread across multiple translation units";

static cl::opt<std::string> TU1Arg(cl::name("tu1"),
                                   cl::init("TU1 argument initial value"));

int main(int argc, const char **argv) {
  if (!cl::ParseArgs(argc, argv))
    cl::PrintHelp(TOOLNAME, TOOLDESC, std::cout);
  std::cout << *TU1Arg << "\n" << getTU2Arg() << "\n";
}
