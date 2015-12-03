#include "motis/loader/gtfs/agency.h"

#include <tuple>
#include <algorithm>

#include "parser/csv.h"

#include "motis/loader/util.h"

using namespace parser;
using namespace flatbuffers;
using std::get;

namespace motis {
namespace loader {
namespace gtfs {

enum { agency_id, agency_name, agency_timezone };
using gtfs_agency = std::tuple<cstr, cstr, cstr>;
static const column_mapping<gtfs_agency> columns = {
    {"agency_id", "agency_name", "agency_timezone"}};

agency_map read_agencies(loaded_file file) {
  agency_map agencies;
  for (auto const& a : read<gtfs_agency>(file.content(), columns)) {
    agencies.emplace(get<agency_id>(a).to_str(),
                     make_unique<agency>(get<agency_id>(a).to_str(),
                                         get<agency_name>(a).to_str(),
                                         get<agency_timezone>(a).to_str()));
  }
  return agencies;
}

}  // namespace gtfs
}  // namespace loader
}  // namespace motis
