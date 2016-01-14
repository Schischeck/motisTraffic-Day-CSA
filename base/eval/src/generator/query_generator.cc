#include <iostream>
#include <fstream>
#include <random>

#include "boost/program_options.hpp"

#include "boost/date_time/gregorian/gregorian_types.hpp"
#include "boost/date_time/posix_time/posix_time.hpp"

#include "conf/options_parser.h"

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

std::string query(std::time_t interval_start, std::time_t interval_end,
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
  std::string s = make_msg(fbb)->to_json();
  s.erase(std::remove(begin(s), end(s), '\n'), end(s));
  return s;
}

int dep_event_sum(station const& s) {
  return std::accumulate(begin(s.dep_class_events), end(s.dep_class_events), 0);
}

int arr_event_sum(station const& s) {
  return std::accumulate(begin(s.arr_class_events), end(s.arr_class_events), 0);
}

station* random_station(std::vector<station_ptr> const& stations) {
  auto first = std::next(begin(stations), 1);
  auto last = std::next(begin(stations), stations.size() - 1);
  return rand_in(first, last)->get();
}

std::pair<std::string, std::string> random_eva_pair(
    std::vector<station_ptr> const& stations) {
  std::pair<station*, station*> p = std::make_pair(nullptr, nullptr);
  while (p.first == p.second || dep_event_sum(*p.first) == 0 ||
         arr_event_sum(*p.second) == 0) {
    p.first = random_station(stations);
    p.second = random_station(stations);
  }
  return std::make_pair(p.first->eva_nr, p.second->eva_nr);
}

int main(int argc, char** argv) {
  dataset_settings dataset_opt("rohdaten", true, false, false, "TODAY", 2);
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
  int total_seconds = sched.schedule_end_ - sched.schedule_begin_ -
                      (SCHEDULE_OFFSET_MINUTES * 60);

  for (int i = 0; i < generator_opt.query_count; ++i) {
    auto interval_start = sched.schedule_begin_ + SCHEDULE_OFFSET_MINUTES * 60 +
                          rand_in(0, total_seconds);
    auto interval_end = interval_start + 3600;
    auto evas = random_eva_pair(sched.stations);
    out << query(interval_start, interval_end, evas.first, evas.second) << "\n";
  }
}
