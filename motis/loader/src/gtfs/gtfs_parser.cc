#include "motis/loader/parsers/gtfs/gtfs_parser.h"

namespace motis {
namespace loader {
namespace gtfs {

bool gtfs_parser::applicable(std::string const&) { return true; }

void gtfs_parser::parse(std::string const&) {}

}  // gtfs
}  // loader
}  // motis
