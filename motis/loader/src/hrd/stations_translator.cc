#include "motis/loader/parsers/hrd/stations_translator.h"

#include "parser/arg_parser.h"

#include "motis/core/common/logging.h"
#include "motis/loader/util.h"
#include "motis/loader/parser_error.h"
#include "motis/loader/parsers/hrd/files.h"

using namespace parser;
using namespace flatbuffers;
using namespace motis::logging;

namespace motis {
namespace loader {
namespace hrd {

Offset<Station> stations_translator::get_or_create_station(int eva_num) {
  return get_or_create(fbs_stations_, eva_num, [&]() {
    auto it = stations_.find(eva_num);
    verify(it != end(stations_), "missing station: %d", eva_num);
    return CreateStation(
        builder_, to_fbs_string(builder_, std::to_string(eva_num)),
        to_fbs_string(builder_, it->second.name, ENCODING), it->second.lat,
        it->second.lng, it->second.change_time);
  });
}

}  // hrd
}  // loader
}  // motis
