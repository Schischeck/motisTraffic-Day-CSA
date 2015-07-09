#include "motis/guesser/guesser.h"

#include <iostream>

#include "boost/program_options.hpp"

#include "motis/module/api.h"

#include "motis/protocol/Message_generated.h"

using namespace flatbuffers;
using namespace motis::module;
namespace po = boost::program_options;

namespace motis {
namespace guesser {

po::options_description guesser::desc() {
  po::options_description desc("Guesser Module");
  return desc;
}

void guesser::print(std::ostream& out) const {}

void guesser::init() {
  std::vector<std::string> station_names(schedule_->stations.size());
  std::transform(begin(schedule_->stations), end(schedule_->stations),
                 begin(station_names),
                 [](station_ptr const& s) { return s->name; });
  guesser_ = std::unique_ptr<guess::guesser>(new guess::guesser(station_names));
}

msg_ptr guesser::on_msg(msg_ptr const& msg, sid) {
  auto req = msg->content<StationGuesserRequest const*>();

  FlatBufferBuilder b;

  std::vector<Offset<Station>> guesses;
  for (auto const& guess : guesser_->guess(req->input()->str())) {
    if (guess < 0 || guess >= schedule_->stations.size()) {
      continue;
    }

    auto const& station = *schedule_->stations[guess];
    try {
      guesses.emplace_back(
          CreateStation(b, b.CreateString(station.name),
                        boost::lexical_cast<int>(station.eva_nr), station.width,
                        station.length));
    } catch (boost::bad_lexical_cast const&) {
      continue;
    }
  }

  b.Finish(motis::CreateMessage(
      b, MsgContent_StationGuesserResponse,
      CreateStationGuesserResponse(b, b.CreateVector(guesses)).Union()));

  return make_msg(b);
}

MOTIS_MODULE_DEF_MODULE(guesser)

}  // namespace guesser
}  // namespace motis
