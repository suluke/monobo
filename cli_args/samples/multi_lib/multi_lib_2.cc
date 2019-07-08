#include "cli_args/cli_args.h"
#include "multi_lib.h"

namespace cl = ::cli_args;

static cl::opt<std::string> Lib2Arg(cl::name("lib2"),
                                    cl::init("Lib2 argument initial value"));

const std::string &getLib2Arg() { return Lib2Arg; }