#include "motis/guesser/guesser.h"

#include <iostream>
#include <numeric>

#include "boost/program_options.hpp"

#include "motis/loader/util.h"
#include "motis/protocol/Message_generated.h"

namespace p = std::placeholders;
namespace po = boost::program_options;
using namespace flatbuffers;
using namespace motis::module;

namespace motis {
namespace guesser {

std::string trim(std::string const& s) {
  auto first = s.find_first_not_of(' ');
  auto last = s.find_last_not_of(' ');
  if (first == last) {
    return "";
  } else {
    return s.substr(first, (last - first + 1));
  }
}

po::options_description guesser::desc() {
  po::options_description desc("Guesser Module");
  return desc;
}

void guesser::print(std::ostream&) const {}

void guesser::init(motis::module::registry& reg) {
  auto sync = synced_sched<schedule_access::RO>();

  std::set<std::string> station_names;
  station_indices_ = std::accumulate(
      begin(sync.sched().stations), end(sync.sched().stations),
      std::vector<unsigned>(),
      [&station_names](std::vector<unsigned>& indices, station_ptr const& s) {
        auto total_events = std::accumulate(begin(s->dep_class_events),
                                            end(s->dep_class_events), 0) +
                            std::accumulate(begin(s->arr_class_events),
                                            end(s->arr_class_events), 0);
        if (total_events != 0 && station_names.insert(s->name).second) {
          indices.push_back(s->index);
        }
        return indices;
      });

  auto stations = loader::transform_to_vec(
      begin(station_indices_), end(station_indices_), [&](unsigned i) {
        auto const& s = *sync.sched().stations[i];
        double factor = 0;
        for (unsigned i = 0; i < s.dep_class_events.size(); ++i) {
          factor += std::pow(10, (9 - i) / 3) * s.dep_class_events[i];
        }
        return std::make_pair(s.name, factor);
      });

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
  reg.register_op("/guesser", std::bind(&guesser::guess, this, p::_1));
}

msg_ptr guesser::guess(msg_ptr const& msg) {
  auto req = motis_content(StationGuesserRequest, msg);
  auto sync = synced_sched<schedule_access::RO>();

  MessageCreator b;

  std::vector<Offset<Station>> guesses;
  for (auto const& guess :
       guesser_->guess(trim(req->input()->str()), req->guess_count())) {
    auto const& station = *sync.sched().stations[station_indices_[guess]];
    guesses.emplace_back(CreateStation(
        b, b.CreateString(station.name), b.CreateString(station.eva_nr),
        station.index, station.width, station.length));
  }

  b.CreateAndFinish(
      MsgContent_StationGuesserResponse,
      CreateStationGuesserResponse(b, b.CreateVector(guesses)).Union());

  return make_msg(b);
}

}  // namespace guesser
}  // namespace motis
