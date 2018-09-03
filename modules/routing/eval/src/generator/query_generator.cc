#include <fstream>
#include <iostream>
#include <random>

#include "boost/program_options.hpp"

#include "boost/date_time/gregorian/gregorian_types.hpp"
#include "boost/date_time/posix_time/posix_time.hpp"

#include "utl/erase.h"
#include "utl/to_vec.h"

#include "conf/options_parser.h"

#include "motis/core/schedule/time.h"
#include "motis/core/access/time_access.h"
#include "motis/module/message.h"
#include "motis/bootstrap/dataset_settings.h"
#include "motis/bootstrap/motis_instance.h"

namespace po = boost::program_options;
using namespace flatbuffers;
using namespace motis;
using namespace motis::bootstrap;
using namespace motis::module;
using namespace motis::routing;

#define QUERY_COUNT "query_count"
#define TARGET_FILE_FWD "target_file_fwd"
#define TARGET_FILE_BWD "target_file_bwd"
#define LARGE_STATIONS "large_stations"

class generator_settings : public conf::configuration {
public:
  generator_settings(int query_count, std::string target_file,
                     std::string target_file_bwd, bool large_stations)
      : query_count_(query_count),
        target_file_fwd_(std::move(target_file)),
        target_file_bwd_(std::move(target_file_bwd)),
        large_stations_(large_stations) {}

  ~generator_settings() override = default;

  generator_settings(generator_settings const&) = default;
  generator_settings& operator=(generator_settings const&) = default;

  generator_settings(generator_settings&&) = default;
  generator_settings& operator=(generator_settings&&) = default;

  boost::program_options::options_description desc() override {
    po::options_description desc("Generator Settings");
    // clang-format off
    desc.add_options()
      (QUERY_COUNT,
          po::value<int>(&query_count_)->default_value(query_count_),
          "number of queries to generate")
      (TARGET_FILE_FWD,
          po::value<std::string>(&target_file_fwd_)->default_value(target_file_fwd_),
          "file to write generated queries to")
      (TARGET_FILE_BWD,
          po::value<std::string>(&target_file_bwd_)->default_value(target_file_bwd_),
          "file to write generated queries to")
      (LARGE_STATIONS,
          po::value<bool>(&large_stations_)->default_value(large_stations_),
          "use only large stations");
    // clang-format on
    return desc;
  }

  void print(std::ostream& out) const override {
    out << "  " << QUERY_COUNT << ": " << query_count_ << "\n"
        << "  " << TARGET_FILE_FWD << ": " << target_file_fwd_ << "\n"
        << "  " << TARGET_FILE_BWD << ": " << target_file_bwd_ << "\n"
        << "  " << LARGE_STATIONS << ": " << large_stations_;
  }

  int query_count_;
  std::string target_file_fwd_;
  std::string target_file_bwd_;
  bool large_stations_;
};

struct search_interval_generator {
  search_interval_generator(time_t begin, time_t end)
      : begin_(begin), rng_(rd_()), d_(generate_distribution(begin, end)) {
    rng_.seed(std::time(nullptr));
  }

  std::pair<time_t, time_t> random_interval() {
    auto begin = begin_ + d_(rng_) * 3600;
    return {begin, begin + 3600};
  }

private:
  static std::discrete_distribution<int> generate_distribution(time_t begin,
                                                               time_t end) {
    auto constexpr k_two_hours = 2 * 3600;
    static const int prob[] = {
        1,  // 01: 00:00 - 01:00
        1,  // 02: 01:00 - 02:00
        1,  // 03: 02:00 - 03:00
        1,  // 04: 03:00 - 04:00
        1,  // 05: 04:00 - 05:00
        2,  // 06: 05:00 - 06:00
        3,  // 07: 06:00 - 07:00
        4,  // 08: 07:00 - 08:00
        4,  // 09: 08:00 - 09:00
        3,  // 10: 09:00 - 10:00
        2,  // 11: 10:00 - 11:00
        2,  // 12: 11:00 - 12:00
        2,  // 13: 12:00 - 13:00
        2,  // 14: 13:00 - 14:00
        3,  // 15: 14:00 - 15:00
        4,  // 16: 15:00 - 16:00
        4,  // 17: 16:00 - 17:00
        4,  // 18: 17:00 - 18:00
        4,  // 19: 18:00 - 19:00
        3,  // 20: 19:00 - 20:00
        2,  // 21: 20:00 - 21:00
        1,  // 22: 21:00 - 22:00
        1,  // 23: 22:00 - 23:00
        1  // 24: 23:00 - 24:00
    };
    std::vector<int> v;
    for (time_t t = begin, hour = 0; t < end - k_two_hours; t += 3600, ++hour) {
      int h = hour % 24;
      v.push_back(prob[h]);  // NOLINT
    }
    return std::discrete_distribution<int>(std::begin(v), std::end(v));
  }

  time_t begin_;
  std::random_device rd_;
  std::mt19937 rng_;
  std::vector<int> hour_prob_;
  std::discrete_distribution<int> d_;
};

static int rand_in(int start, int end) {
  static bool initialized = false;
  static std::mt19937 rng;  // NOLINT
  if (!initialized) {
    initialized = true;
    rng.seed(std::time(nullptr));
  }

  std::uniform_int_distribution<int> dist(start, end);
  return dist(rng);
}

template <typename It>
static It rand_in(It begin, It end) {
  return std::next(begin, rand_in(0, std::distance(begin, end) - 1));
}

std::string query(int id, std::time_t interval_start, std::time_t interval_end,
                  std::string const& from_eva, std::string const& to_eva,
                  SearchDir const dir) {
  message_creator fbb;
  auto const interval = Interval(interval_start, interval_end);
  fbb.create_and_finish(
      MsgContent_RoutingRequest,
      CreateRoutingRequest(
          fbb, Start_PretripStart,
          CreatePretripStart(fbb,
                             CreateInputStation(fbb, fbb.CreateString(from_eva),
                                                fbb.CreateString("")),
                             &interval)
              .Union(),
          CreateInputStation(fbb, fbb.CreateString(to_eva),
                             fbb.CreateString("")),
          SearchType_Default, dir, fbb.CreateVector(std::vector<Offset<Via>>()),
          fbb.CreateVector(std::vector<Offset<AdditionalEdgeWrapper>>()))
          .Union(),
      "/routing");
  auto msg = make_msg(fbb);
  msg->get()->mutate_id(id);

  auto json = msg->to_json();
  utl::erase(json, '\n');
  return json;
}

bool has_events(edge const& e, motis::time from, motis::time to) {
  auto[con, day] = e.get_connection(from);
  return con != nullptr && con->event_time(event_type::DEP, day) <= to;
}

bool has_events(station_node const& s, motis::time from, motis::time to) {
  for (auto const& r : s.get_route_nodes()) {
    for (auto const& e : r->edges_) {
      if (!e.empty() && has_events(e, from, to)) {
        return true;
      }
    }
  }
  return false;
}

int random_station_id(std::vector<station_node const*> const& station_nodes,
                      motis::time motis_interval_start,
                      motis::time motis_interval_end) {
  auto first = std::next(begin(station_nodes), 2);
  auto last = end(station_nodes);

  station_node const* s;
  do {
    s = *rand_in(first, last);
  } while (!has_events(*s, motis_interval_start, motis_interval_end));
  return s->id_;
}

std::pair<std::string, std::string> random_station_ids(
    schedule const& sched,
    std::vector<station_node const*> const& station_nodes,
    time_t interval_start, time_t interval_end) {
  std::string from, to;
  auto motis_interval_start = unix_to_motistime(sched, interval_start);
  auto motis_interval_end = unix_to_motistime(sched, interval_end);
  if (motis_interval_start == INVALID_TIME ||
      motis_interval_end == INVALID_TIME) {
    std::cout << "ERROR: generated timestamp not valid:\n";
    std::cout << "  schedule range: " << sched.schedule_begin_ << " - "
              << sched.schedule_end_ << "\n";
    std::cout << "  interval_start = " << interval_start << " ("
              << format_time(motis_interval_start) << ")\n";
    std::cout << "  interval_end = " << interval_end << " ("
              << format_time(motis_interval_end) << ")\n";
    std::terminate();
  }
  do {
    from = sched.stations_
               .at(random_station_id(station_nodes, motis_interval_start,
                                     motis_interval_end))
               ->eva_nr_;
    to = sched.stations_
             .at(random_station_id(station_nodes, motis_interval_start,
                                   motis_interval_end))
             ->eva_nr_;
  } while (from == to);
  return std::make_pair(from, to);
}

int main(int argc, char** argv) {
  dataset_settings dataset_opt("rohdaten", "TODAY", 2, true, false, false,
                               false);
  generator_settings generator_opt(1000, "queries_fwd.txt", "queries_bwd.txt",
                                   false);

  conf::options_parser parser({&dataset_opt, &generator_opt});
  parser.read_command_line_args(argc, argv);

  if (parser.help()) {
    std::cout << "\n\tQuery Generator\n\n";
    parser.print_help(std::cout);
    return 0;
  } else if (parser.version()) {
    std::cout << "Query Generator\n";
    return 0;
  }

  parser.read_configuration_file();

  std::cout << "\n\tQuery Generator\n\n";
  parser.print_unrecognized(std::cout);
  parser.print_used(std::cout);

  std::ofstream out_fwd(generator_opt.target_file_fwd_);
  std::ofstream out_bwd(generator_opt.target_file_bwd_);

  motis_instance instance;
  instance.init_schedule(dataset_opt);

  auto const& sched = *instance.schedule_;
  search_interval_generator interval_gen(sched.loaded_begin_,
                                         sched.loaded_end_);

  std::vector<station_node const*> station_nodes;
  if (generator_opt.large_stations_) {
    auto const num_stations = 1000;

    auto stations = utl::to_vec(sched.stations_,
                                [](station_ptr const& s) { return s.get(); });

    std::vector<double> sizes(stations.size());
    for (auto i = 0u; i < stations.size(); ++i) {
      auto& factor = sizes[i];
      auto const& events = stations[i]->dep_class_events_;
      for (unsigned i = 0; i < events.size(); ++i) {
        factor += std::pow(10, (9 - i) / 3) * events.at(i);
      }
    }

    std::sort(begin(stations), end(stations),
              [&](station const* a, station const* b) {
                return sizes[a->index_] > sizes[b->index_];
              });

    auto const n = std::min(static_cast<size_t>(num_stations), stations.size());
    for (auto i = 0u; i < n; ++i) {
      station_nodes.push_back(
          sched.station_nodes_.at(stations[i]->index_).get());
    }
  } else {
    station_nodes =
        utl::to_vec(sched.station_nodes_, [](station_node_ptr const& s) {
          return static_cast<station_node const*>(s.get());
        });
  }

  for (int i = 1; i <= generator_opt.query_count_; ++i) {
    auto interval = interval_gen.random_interval();
    auto evas = random_station_ids(sched, station_nodes, interval.first,
                                   interval.second);
    out_fwd << query(i, interval.first, interval.second, evas.first,
                     evas.second, SearchDir_Forward)
            << "\n";
    out_bwd << query(i, interval.first, interval.second, evas.first,
                     evas.second, SearchDir_Backward)
            << "\n";
  }
}
