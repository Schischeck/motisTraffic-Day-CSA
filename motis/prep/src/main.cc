#include <ostream>
#include <string>

#include "boost/date_time/posix_time/posix_time.hpp"
#include "boost/program_options.hpp"
#include "boost/io/ios_state.hpp"

#include "conf/options_parser.h"

#include "motis/core/common/logging.h"

#include "motis/loader/loader.h"

#include "motis/prep/services2routes.h"
#include "motis/prep/serialize.h"
#include "motis/prep/station_graph.h"

#define SCHEDULE_PREFIX ("dataset")
#define SERVICES_TO_ROUTES ("services_to_routes")
#define SERIALIZE_GRAPH ("serialize_graph")
#define WRITE_STATION_GRAPH ("station_graph")

using namespace motis;
using namespace motis::logging;
namespace po = boost::program_options;

class serialize_settings : public conf::configuration {
public:
  serialize_settings(std::string default_schedule_prefix, bool default_s2r,
                     bool default_station_graph, bool default_serialize)
      : schedule_prefix(default_schedule_prefix),
        s2r(default_s2r),
        station_graph(default_station_graph),
        serialize(default_serialize) {}

  boost::program_options::options_description desc() {
    po::options_description desc("callback options");
    // clang-format off
    desc.add_options()
      (SCHEDULE_PREFIX,
          po::value<std::string>(&schedule_prefix)->default_value(schedule_prefix),
          "path to the schedule (not including the last '.')\n")
      (SERVICES_TO_ROUTES,
          po::bool_switch(&s2r)->default_value(s2r),
          "services to routes transform step\n"
          "\n"
          "creates files:\n"
          "- PREFIX.Routes.txt\n"
          "- PREFIX.Route.Bitfields.txt\n")
      (WRITE_STATION_GRAPH,
          po::bool_switch(&station_graph)->default_value(station_graph),
          "write file containing the station graph "
          "(elementary connections)\n"
          "\n"
          "creates files:\n"
          "- PREFIX.sgrv.csv\n")
      (SERIALIZE_GRAPH,
          po::bool_switch(&serialize)->default_value(serialize),
          "graph serialization step\n"
          "\n"
          "creates files:\n"
          "- PREFIX.Schedule.raw\n"
          "- PREFIX.Schedule.index\n");
    // clang-format on
    return desc;
  }

  void print(std::ostream& out) const {
    using std::boolalpha;
    boost::io::ios_all_saver guard(out);
    out << "  " << SCHEDULE_PREFIX << ": " << schedule_prefix << "\n"
        << "  " << SERVICES_TO_ROUTES << ": " << boolalpha << s2r << "\n"
        << "  " << SERIALIZE_GRAPH << ": " << boolalpha << serialize << "\n";
  }

  std::string schedule_prefix;
  bool s2r, station_graph, serialize;
};

int main(int argc, char* argv[]) {
  serialize_settings serialize_opt("schedule/test", false, false, false);
  conf::options_parser parser({&serialize_opt});
  parser.read_command_line_args(argc, argv);

  if (parser.help()) {
    std::cout << "\n\tMOTIS serialize\n\n";
    parser.print_help(std::cout);
    return 0;
  } else if (parser.version()) {
    std::cout << "MOTIS serialize\n";
    return 0;
  }

  parser.read_configuration_file();

  std::cout << "\n\tMOTIS serialize\n\n";
  parser.print_unrecognized(std::cout);
  parser.print_used(std::cout);

  scoped_timer timer("all_steps");

  if (serialize_opt.s2r) {
    scoped_timer timer(SERVICES_TO_ROUTES);
    services2routes(serialize_opt.schedule_prefix);
  }

  schedule_ptr schedule;
  if (serialize_opt.serialize || serialize_opt.station_graph) {
    scoped_timer timer("reading_schedule");
    if (serialize_opt.serialize)
      schedule = load_text_schedule(serialize_opt.schedule_prefix);
    else
      schedule = load_binary_schedule(serialize_opt.schedule_prefix);
  }

  if (serialize_opt.station_graph) {
    scoped_timer timer(WRITE_STATION_GRAPH);
    write_station_graph(*schedule, serialize_opt.schedule_prefix);
  }

  if (serialize_opt.serialize) {
    scoped_timer timer(SERIALIZE_GRAPH);
    serialize(*reinterpret_cast<text_schedule*>(schedule.get()),
              serialize_opt.schedule_prefix);
  }
}
