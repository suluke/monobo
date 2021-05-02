#ifndef CLI_ARGS_CLI_ARGS_HELP_H
#define CLI_ARGS_CLI_ARGS_HELP_H

#include <algorithm>
#include <iomanip>
#include <ostream>
#include <vector>

namespace cli_args {
/// TODO Allow influencing display order
template <typename AppTag = void>
inline void PrintHelp(const char *tool, const char *desc, std::ostream &os) {
  using namespace ::cli_args::detail;
  const CliOptRegistry<AppTag> &registry = CliOptRegistry<AppTag>::get();
  const auto display = [](const detail::CliOptConcept &opt, std::ostream &os) {
    if (!opt.getNames().empty() && opt.getNames().at(0) != "") {
      const detail::CliOptConcept::string_span names = opt.getNames();
      const auto nameBegin = names.begin(), nameEnd = names.end();
      unsigned firstColWidth = 0,
               maxFirstColWidth = 18; // FIXME this is a pretty arbitrary value
      for (auto nameIt = nameBegin; nameIt != nameEnd; ++nameIt) {
        if (nameIt != nameBegin) {
          os << ", ";
          firstColWidth += 2;
        }
        if (nameIt->size() == 1)
          os << "-";
        else
          os << "--";
        os << *nameIt;
        firstColWidth += 1 + nameIt->size() + (nameIt->size() > 1);
      }
      std::ios_base::fmtflags defaultFmtFlags(os.flags());
      os << std::setw(maxFirstColWidth - firstColWidth) << "" << std::setw(0)
         << opt.getDescription();
      os.flags(defaultFmtFlags);
    } else
      os << opt.getMeta();
  };
  // FIXME this is still in MVP state
  os << "Usage: " << tool << " [OPTION]...";
  const CliOptConcept *eatAll = nullptr;
  if (registry.hasUnnamed()) {
    eatAll = &registry.getUnnamed();
    os << " ";
    if (!eatAll->isRequired())
      os << "[";
    display(*eatAll, os);
    if (!eatAll->isRequired())
      os << "]";
    os << "...";
  }
  os << "\n"
     << "Options:\n";
  std::vector<CliOptConcept *> optsSorted(registry.begin(), registry.end());
  std::sort(optsSorted.begin(), optsSorted.end(),
            [](CliOptConcept *A, CliOptConcept *B) {
              return A->getShortestName().compare(B->getShortestName()) < 0;
            });
  for (const CliOptConcept *opt : optsSorted) {
    if (opt == eatAll)
      continue;
    os << "  ";
    display(*opt, os);
    os << '\n';
  }
}
} // namespace cli_args
#endif // CLI_ARGS_CLI_ARGS_HELP_H
