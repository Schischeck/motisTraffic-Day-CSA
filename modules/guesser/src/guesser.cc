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

  auto stations = view::all(station_indices_) | view::transform([&](int i) {
                    auto const& s = *sync.sched().stations[i];
                    double factor = 0;
                    for (unsigned i = 0; i < s.dep_class_events.size(); ++i) {
                      factor +=
                          std::pow(10, (9 - i) / 3) * s.dep_class_events[i];
                    }
                    return std::make_pair(s.name, factor);
                  }) |
                  to_vector;

  if (!stations.empty()) {
    auto max_importatance =
        std::max_element(begin(stations), end(stations),
                         [](std::pair<std::string, double> const& lhs,
                            std::pair<std::string, double> const& rhs) {
                           return lhs.second < rhs.second;
                         })
            ->second;
    for (auto& s : stations) {
      s.second = (s.second / max_importatance) * 0.5;
    }
  }

  guesser_ = std::unique_ptr<guess::guesser>(new guess::guesser(stations));
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
