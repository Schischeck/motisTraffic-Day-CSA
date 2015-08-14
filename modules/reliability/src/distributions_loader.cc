#include "motis/reliability/distributions_loader.h"

namespace motis {
namespace reliability {

/* external lib:
 * https://github.com/felixguendling/parser/blob/bd2df0e6da37ee116483223d17ba505d14393846/include/parser/arg_parser.h
 * example:
 * https://algotud.codebasehq.com/projects/motis/repositories/td/blob/parser/motis/loader/src/gtfs/stop.cc
 *
 * TODO: read csv (original bahn format) and sort the entries
 */

namespace distributions_loader {
void load_distributions(std::string const root) {}

namespace detail {

void load_distributions_classes(std::string const file) {}

void load_distribution_mappings(std::string const file) {}

void load_distributions(std::string const file) {}

void load_start_distributions(std::string const file) {}

}  // namespace detail
}  // namespace distributions_loader

}  // namespace reliability
}  // namespace motis
