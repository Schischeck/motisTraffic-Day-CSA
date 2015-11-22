#include "motis/guesser/guesser.h"

#include <iostream>

#include "range/v3/view/transform.hpp"
#include "range/v3/view/remove_if.hpp"
#include "range/v3/numeric/accumulate.hpp"

#include "boost/program_options.hpp"

#include "motis/protocol/Message_generated.h"

using namespace ranges;
using namespace flatbuffers;
using namespace motis::module;
namespace po = boost::program_options;

namespace motis {
namespace guesser {

po::options_description guesser::desc() {
  po::options_description desc("Guesser Module");
  return desc;
}

void guesser::print(std::ostream&) const {}

void guesser::init() {
  auto sync = synced_sched<schedule_access::RO>();

  station_indices_ =
      view::all(sync.sched().stations) |
      view::remove_if([](station_ptr const& s) {
        return accumulate(view::all(s->dep_class_events), 0) == 0;
      }) |
      view::transform([](station_ptr const& s) { return s->index; }) |
      to_vector;

  guesser_ = std::unique_ptr<guess::guesser>(new guess::guesser(
      view::all(station_indices_) |
      view::transform([&](int i) { return sync.sched().stations[i]->name; }) |
      to_vector));
}

void guesser::on_msg(msg_ptr msg, sid, callback cb) {
  auto req = msg->content<StationGuesserRequest const*>();

  auto sync = synced_sched<schedule_access::RO>();

  MessageCreator b;

  std::vector<Offset<Station>> guesses;
  for (auto const& guess :
       guesser_->guess(req->input()->str(), req->guess_count())) {
    auto const& station = *sync.sched().stations[station_indices_[guess]];
    guesses.emplace_back(CreateStation(
        b, b.CreateString(station.name), b.CreateString(station.eva_nr),
        station.index, station.width, station.length));
  }

  b.CreateAndFinish(
      MsgContent_StationGuesserResponse,
      CreateStationGuesserResponse(b, b.CreateVector(guesses)).Union());

  return cb(make_msg(b), boost::system::error_code());
}

}  // namespace guesser
}  // namespace motis
