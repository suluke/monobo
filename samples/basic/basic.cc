#include "cli_args/cli_args.h"

namespace cl = ::cli_args;

const char *const TOOLNAME = "basic";
const char *const TOOLDESC = "Basic cli_args usage demo";

static cl::list<std::string> AllArgs(cl::meta("all argument strings"),
                                     cl::init({"There", "were", "no", "args"}));

int main(int argc, const char **argv) {
  if (!cl::ParseArgs(argc, argv))
    cl::PrintHelp(TOOLNAME, TOOLDESC, std::cout);
  for (const std::string &arg : AllArgs)
    std::cout << arg << "\n";
  return 0;
}
