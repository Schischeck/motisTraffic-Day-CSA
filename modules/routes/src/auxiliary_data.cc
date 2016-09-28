#include "motis/routes/auxiliary_data.h"

#include "boost/filesystem.hpp"

#include "parser/file.h"

#include "motis/core/common/logging.h"
#include "motis/core/common/transform_to_vec.h"
#include "motis/module/message.h"

#include "motis/routes/fbs/RoutesAuxiliary_generated.h"

using namespace flatbuffers;
using namespace parser;
using namespace motis::geo;
using namespace motis::module;

namespace motis {
namespace routes {

void auxiliary_data::load(std::string const& filename) {
  if (!boost::filesystem::is_regular_file(filename)) {
    return;
  }

  logging::scoped_timer timer("loading auxiliary routes data");

  auto const buf = file(filename.c_str(), "r").content();
  auto const aux_content = GetRoutesAuxiliary(buf.buf_);

  for (auto const& s : *aux_content->bus_stop_positions()) {
    extra_stop_positions_[s->station_id()->str()] =
        transform_to_vec(*s->positions(), [](auto&& p) {
          return latlng{p->lat(), p->lng()};
        });
  }

  for (auto const& route : *aux_content->routes()) {
    message_creator mc;
    auto const segments =
        transform_to_vec(*route->segments(), [&mc](auto const& s) {
          return motis::CreatePolyline(
              mc, mc.CreateVector(s->coordinates()->data(),
                                  s->coordinates()->size()));
        });

    mc.create_and_finish(
        MsgContent_RoutesIdTrainResponse,
        CreateRoutesIdTrainResponse(mc, mc.CreateVector(segments),
                                    mc.CreateString("osm"))
            .Union());

    auto const seq = transform_to_vec(*route->station_ids(),
                                      [](auto const& id) { return id->str(); });

    auto msg = make_msg(mc);
    for (auto const& clasz : *route->classes()) {
      prepared_routes_[{seq, clasz}] = msg;
    }
  }
}

}  // namespace routes
}  // namespace motis
