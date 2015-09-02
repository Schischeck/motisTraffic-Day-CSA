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
  auto sync = synced_sched<schedule_access::RO>();
  std::vector<std::string> station_names(sync.sched().stations.size());
  std::transform(begin(sync.sched().stations), end(sync.sched().stations),
                 begin(station_names),
                 [](station_ptr const& s) { return s->name; });
  guesser_ = std::unique_ptr<guess::guesser>(new guess::guesser(station_names));
}

void guesser::on_msg(msg_ptr msg, sid, callback cb) {
  auto req = msg->content<StationGuesserRequest const*>();

  auto sync = synced_sched<schedule_access::RO>();

  FlatBufferBuilder b;

  std::vector<Offset<Station>> guesses;
  for (auto const& guess :
       guesser_->guess(req->input()->str(), req->guess_count())) {
    auto const& station = *sync.sched().stations[guess];
    guesses.emplace_back(CreateStation(b, b.CreateString(station.name),
                                       station.eva_nr, station.index, station.width,
                                       station.length));
  }

  b.Finish(motis::CreateMessage(
      b, MsgContent_StationGuesserResponse,
      CreateStationGuesserResponse(b, b.CreateVector(guesses)).Union()));

  return cb(make_msg(b), boost::system::error_code());
}

MOTIS_MODULE_DEF_MODULE(guesser)

}  // namespace guesser
}  // namespace motis
