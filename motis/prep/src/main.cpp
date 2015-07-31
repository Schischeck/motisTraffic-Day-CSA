#include <ostream>
#include <string>

#include "boost/date_time/posix_time/posix_time.hpp"
#include "boost/program_options.hpp"
#include "boost/io/ios_state.hpp"

#include "conf/options_parser.h"

#include "motis/core/common/Logging.h"

#include "motis/loader/Loader.h"

#include "motis/prep/Services2Routes.h"
#include "motis/prep/Serialize.h"
#include "motis/prep/StationGraph.h"

#define SCHEDULE_PREFIX       ("dataset")
#define SERVICES_TO_ROUTES    ("services_to_routes")
#define SERIALIZE_GRAPH       ("serialize_graph")
#define WRITE_STATION_GRAPH   ("station_graph")

using namespace td;
using namespace td::logging;
namespace po = boost::program_options;

class SerializeSettings : public conf::configuration
{
public:
  SerializeSettings(std::string defaultSchedulePrefix,
                    bool defaultS2r,
                    bool defaultStationGraph,
                    bool defaultSerialize)
      : schedulePrefix(defaultSchedulePrefix),
        s2r(defaultS2r),
        stationGraph(defaultStationGraph),
        serialize(defaultSerialize)
  {}

  boost::program_options::options_description desc()
  {
    po::options_description desc("Callback Options");
    desc.add_options()
      (SCHEDULE_PREFIX,
          po::value<std::string>(&schedulePrefix)->default_value(schedulePrefix),
          "path to the schedule (not including the last '.')\n")
      (SERVICES_TO_ROUTES,
          po::bool_switch(&s2r)->default_value(s2r),
          "services to routes transform step\n"
          "\n"
          "creates files:\n"
          "- PREFIX.Routes.txt\n"
          "- PREFIX.Route.Bitfields.txt\n")
      (WRITE_STATION_GRAPH,
          po::bool_switch(&stationGraph)->default_value(stationGraph),
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
    return desc;
  }

  void print(std::ostream& out) const
  {
    using std::boolalpha;
    boost::io::ios_all_saver guard(out);
    out << "  " << SCHEDULE_PREFIX << ": " << schedulePrefix << "\n"
        << "  " << SERVICES_TO_ROUTES << ": " << boolalpha << s2r << "\n"
        << "  " << SERIALIZE_GRAPH << ": " << boolalpha << serialize << "\n";
  }

  std::string schedulePrefix;
  bool s2r, stationGraph, serialize;
};

int main(int argc, char* argv[])
{
  SerializeSettings serializeOpt("schedule/test", false, false, false);
  conf::options_parser parser({ &serializeOpt });
  parser.read_command_line_args(argc, argv);

  if (parser.help())
  {
    std::cout << "\n\tTD Serialize\n\n";
    parser.print_help(std::cout);
    return 0;
  }
  else if (parser.version())
  {
    std::cout << "TD Serialize\n";
    return 0;
  }

  parser.read_configuration_file();

  std::cout << "\n\tTD Serialize\n\n";
  parser.print_unrecognized(std::cout);
  parser.print_used(std::cout);

  ScopedTimer timer("all_steps");

  if (serializeOpt.s2r)
  {
    ScopedTimer timer(SERVICES_TO_ROUTES);
    services2Routes(serializeOpt.schedulePrefix);
  }

  SchedulePtr schedule;
  if (serializeOpt.serialize || serializeOpt.stationGraph)
  {
    ScopedTimer timer("reading_schedule");
    if (serializeOpt.serialize)
      schedule = loadTextSchedule(serializeOpt.schedulePrefix);
    else
      schedule = loadBinarySchedule(serializeOpt.schedulePrefix);
  }

  if (serializeOpt.stationGraph)
  {
    ScopedTimer timer(WRITE_STATION_GRAPH);
    writeStationGraph(*schedule, serializeOpt.schedulePrefix);
  }

  if (serializeOpt.serialize)
  {
    ScopedTimer timer(SERIALIZE_GRAPH);
    serialize(*reinterpret_cast<TextSchedule*>(schedule.get()),
              serializeOpt.schedulePrefix);
  }
}