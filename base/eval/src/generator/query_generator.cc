#include <iostream>
#include <fstream>
#include <random>

#include "boost/program_options.hpp"

#include "boost/date_time/gregorian/gregorian_types.hpp"
#include "boost/date_time/posix_time/posix_time.hpp"

#include "conf/options_parser.h"

#include "motis/core/schedule/time.h"
#include "motis/bootstrap/motis_instance.h"
#include "motis/bootstrap/dataset_settings.h"
#include "motis/module/message.h"

namespace po = boost::program_options;
namespace pt = boost::posix_time;
using namespace flatbuffers;
using namespace motis;
using namespace motis::bootstrap;
using namespace motis::module;
using namespace motis::routing;

#define QUERY_COUNT "query_count"
#define TARGET_FILE "target_file"

class generator_settings : public conf::configuration {
public:
  generator_settings(int query_count, std::string target_file)
      : query_count(query_count), target_file(target_file) {}

  virtual ~generator_settings() {}

  virtual boost::program_options::options_description desc() override {
    po::options_description desc("Generator Settings");
    // clang-format off
    desc.add_options()
      (QUERY_COUNT,
          po::value<int>(&query_count)->default_value(query_count),
          "number of queries to generate")
      (TARGET_FILE,
          po::value<std::string>(&target_file)->default_value(target_file),
          "file to write generated queries to");
    // clang-format on
    return desc;
  }

  virtual void print(std::ostream& out) const override {
    out << "  " << QUERY_COUNT << ": " << query_count << "\n"
        << "  " << TARGET_FILE << ": " << target_file;
  }

  int query_count;
  std::string target_file;
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
    auto constexpr kTwoHours = 2 * 3600;
    static const int prob[] = {
        0,  // 01: 00:00 - 01:00
        0,  // 02: 01:00 - 02:00
        0,  // 03: 02:00 - 03:00
        0,  // 04: 03:00 - 04:00
        0,  // 05: 04:00 - 05:00
        1,  // 06: 05:00 - 06:00
        2,  // 07: 06:00 - 07:00
        3,  // 08: 07:00 - 08:00
        3,  // 09: 08:00 - 09:00
        2,  // 10: 09:00 - 10:00
        1,  // 11: 10:00 - 11:00
        1,  // 12: 11:00 - 12:00
        1,  // 13: 12:00 - 13:00
        1,  // 14: 13:00 - 14:00
        2,  // 15: 14:00 - 15:00
        3,  // 16: 15:00 - 16:00
        3,  // 17: 16:00 - 17:00
        3,  // 18: 17:00 - 18:00
        3,  // 19: 18:00 - 19:00
        2,  // 20: 19:00 - 20:00
        1,  // 21: 20:00 - 21:00
        0,  // 22: 21:00 - 22:00
        0,  // 23: 22:00 - 23:00
        0  // 24: 23:00 - 24:00
    };
    std::vector<int> v;
    for (time_t t = begin, hour = 0; t < end - kTwoHours; t += 3600, ++hour) {
      int h = hour % 24;
      v.push_back(prob[h]);
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
  static std::mt19937 rng;
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
                  std::string const& from_eva, std::string const& to_eva) {
  MessageCreator fbb;
  Interval interval(interval_start, interval_end);

  std::vector<Offset<StationPathElement>> path;
  path.push_back(CreateStationPathElement(fbb, fbb.CreateString(""),
                                          fbb.CreateString(from_eva)));
  path.push_back(CreateStationPathElement(fbb, fbb.CreateString(""),
                                          fbb.CreateString(to_eva)));
  fbb.CreateAndFinish(
      MsgContent_RoutingRequest,
      CreateRoutingRequest(fbb, &interval, Type_PreTrip, Direction_Forward,
                           fbb.CreateVector(path))
          .Union());
  auto msg = make_msg(fbb);
  msg->get()->mutate_id(id);

  std::string s = msg->to_json();
  s.erase(std::remove(begin(s), end(s), '\n'), end(s));
  return s;
}

bool has_events(edge const& e, motis::time from, motis::time to) {
  auto con = e.get_connection(from);
  return con != nullptr && con->d_time <= to;
}

bool has_events(station_node const& s, motis::time from, motis::time to) {
  for (auto const& r : s.get_route_nodes()) {
    for (auto const& e : r->_edges) {
      if (!e.empty() && has_events(e, from, to)) {
        return true;
      }
    }
  }
  return false;
}

std::string random_station_id(schedule const& sched, time_t interval_start,
                              time_t interval_end) {
  auto first = std::next(begin(sched.station_nodes), 2);
  auto last = end(sched.station_nodes);

  auto motis_interval_start =
      unix_to_motistime(sched.schedule_begin_, interval_start);
  auto motis_interval_end =
      unix_to_motistime(sched.schedule_begin_, interval_end);

  station_node const* s;
  do {
    s = rand_in(first, last)->get();
  } while (!has_events(*s, motis_interval_start, motis_interval_end));
  return sched.stations[s->_id]->eva_nr;
}

std::pair<std::string, std::string> random_station_ids(schedule const& sched,
                                                       time_t interval_start,
                                                       time_t interval_end) {
  std::string from, to;
  do {
    from = random_station_id(sched, interval_start, interval_end);
    to = random_station_id(sched, interval_start, interval_end);
  } while (from == to);
  return std::make_pair(from, to);
}

int main(int argc, char** argv) {
  dataset_settings dataset_opt("rohdaten", true, false, false, false, "TODAY",
                               2);
  generator_settings generator_opt(1000, "queries.txt");

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

  std::ofstream out(generator_opt.target_file);

  boost::asio::io_service ios;
  motis_instance instance(&ios);
  instance.init_schedule(dataset_opt);

  auto const& sched = *instance.schedule_;
  search_interval_generator interval_gen(
      sched.schedule_begin_ + SCHEDULE_OFFSET_MINUTES * 60,
      sched.schedule_end_);

  for (int i = 1; i <= generator_opt.query_count; ++i) {
    auto interval = interval_gen.random_interval();
    auto evas = random_station_ids(sched, interval.first, interval.second);
    out << query(i, interval.first, interval.second, evas.first, evas.second)
        << "\n";
  }
}
