#include "cli_args/cli_args.h"
#include "multi_tu.h"

namespace cl = ::cli_args;

static cl::opt<std::string> TU2Arg(cl::name("tu2"),
                                   cl::init("TU2 argument initial value"));

const std::string &getTU2Arg() { return TU2Arg; }