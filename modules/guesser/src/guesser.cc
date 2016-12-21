#include "motis/guesser/guesser.h"

#include <iostream>
#include <numeric>

#include "boost/program_options.hpp"

#include "utl/to_vec.h"

#include "motis/core/common/hash_set.h"
#include "motis/module/context/get_schedule.h"
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
  auto& sched = synced_sched<RO>().sched();

  hash_set<std::string> station_names;
  station_names.set_empty_key("");
  for (auto const& s : sched.stations_) {
    auto total_events = std::accumulate(begin(s->dep_class_events_),
                                        end(s->dep_class_events_), 0) +
                        std::accumulate(begin(s->arr_class_events_),
                                        end(s->arr_class_events_), 0);
    if (total_events != 0 && station_names.insert(s->name_).second) {
      station_indices_.push_back(s->index_);
    }
  }

  auto stations = utl::to_vec(station_indices_, [&](unsigned i) {
    auto const& s = *sched.stations_[i];
    double factor = 0;
    for (unsigned i = 0; i < s.dep_class_events_.size(); ++i) {
      factor += std::pow(10, (9 - i) / 3) * s.dep_class_events_.at(i);
    }
    return std::make_pair(s.name_, factor);
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
      s.second = 1 + (s.second / max_importatance) * 0.5;
    }
  }

  guesser_ = std::make_unique<guess::guesser>(stations);
  reg.register_op("/guesser", std::bind(&guesser::guess, this, p::_1));
}

msg_ptr guesser::guess(msg_ptr const& msg) {
  auto req = motis_content(StationGuesserRequest, msg);

  message_creator b;
  std::vector<Offset<Station>> guesses;
  for (auto const& guess :
       guesser_->guess(trim(req->input()->str()), req->guess_count())) {
    auto const& station = *get_schedule().stations_[station_indices_[guess]];
    auto const pos = Position(station.width_, station.length_);
    guesses.emplace_back(CreateStation(b, b.CreateString(station.eva_nr_),
                                       b.CreateString(station.name_), &pos));
  }

  b.create_and_finish(
      MsgContent_StationGuesserResponse,
      CreateStationGuesserResponse(b, b.CreateVector(guesses)).Union());

  return make_msg(b);
}

}  // namespace guesser
}  // namespace motis
