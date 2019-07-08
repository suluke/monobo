#include "cli_args/cli_args.h"
#include "multi_lib.h"

namespace cl = ::cli_args;

static cl::opt<std::string> Lib1Arg(cl::name("lib1"),
                                    cl::init("Lib1 argument initial value"));

const std::string &getLib1Arg() { return Lib1Arg; }