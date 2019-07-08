#include "cli_args/cli_args.h"
#include "cli_args/parsers/bool.h"

namespace cl = ::cli_args;

const char *const TOOLNAME = "basic";
const char *const TOOLDESC = "Basic cli_args usage demo";

static cl::opt<bool> ABoolOpt(cl::name("b"), cl::name("bool"),
                              cl::desc("An option taking a boolean value"));
static cl::opt<unsigned>
    ARequiredIntOpt(cl::name("i"), cl::name("int"), cl::required(),
                    cl::desc("An option taking an integer value"));
static cl::list<std::string>
    UnnamedArgs(cl::meta("all argument strings"),
                cl::init({"There", "were", "no", "unnamed", "args"}));

int main(int argc, const char **argv) {
  if (!cl::ParseArgs(argc, argv)) {
    cl::PrintHelp(TOOLNAME, TOOLDESC, std::cout);
    return 1;
  }
  std::cout << "The boolean option's value is " << (ABoolOpt ? "true" : "false")
            << "\n"
            << "The integer option's value is " << ARequiredIntOpt << "\n"
            << "Unnamed option values:\n";
  for (const std::string &arg : UnnamedArgs)
    std::cout << arg << "\n";
  return 0;
}
