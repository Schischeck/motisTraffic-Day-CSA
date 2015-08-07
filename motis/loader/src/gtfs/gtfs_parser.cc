#include "motis/loader/parsers/gtfs/gtfs_parser.h"

namespace fs = boost::filesystem;

namespace motis {
namespace loader {
namespace gtfs {

bool gtfs_parser::applicable(fs::path const&) { return true; }

void gtfs_parser::parse(fs::path const&) {}

}  // gtfs
}  // loader
}  // motis
