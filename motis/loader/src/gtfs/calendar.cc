#include "motis/loader/parsers/gtfs/calendar.h"

#include "parser/csv.h"

#include "motis/loader/util.h"

using namespace parser;
using std::get;

namespace motis {
namespace loader {
namespace gtfs {

using gtfs_calendar =
    std::tuple<cstr, int, int, int, int, int, int, int, int, int>;
enum {
  service_id,
  monday,
  tuesday,
  wednesday,
  thursday,
  friday,
  saturday,
  sunday,
  start_date,
  end_date
};

static const column_mapping<gtfs_calendar> calendar_columns = {
    {"service_id", "monday", "tuesday", "wednesday", "thursday", "friday",
     "saturday", "sunday", "start_date", "end_date"}};

std::bitset<7> traffic_week_days(gtfs_calendar const& c) {
  std::bitset<7> days;
  days.set(0, get<sunday>(c) == 1 ? true : false);
  days.set(1, get<monday>(c) == 1 ? true : false);
  days.set(2, get<tuesday>(c) == 1 ? true : false);
  days.set(3, get<wednesday>(c) == 1 ? true : false);
  days.set(4, get<thursday>(c) == 1 ? true : false);
  days.set(5, get<friday>(c) == 1 ? true : false);
  days.set(6, get<saturday>(c) == 1 ? true : false);
  return days;
}

std::map<std::string, calendar> read_calendar(loaded_file file) {
  std::map<std::string, calendar> services;
  for (auto const& c : read<gtfs_calendar>(file.content, calendar_columns)) {
    services.insert(std::make_pair(get<service_id>(c).to_str(),
                                   calendar{traffic_week_days(c),
                                            {yyyymmdd_year(get<start_date>(c)),
                                             yyyymmdd_month(get<start_date>(c)),
                                             yyyymmdd_day(get<start_date>(c))},
                                            {yyyymmdd_year(get<end_date>(c)),
                                             yyyymmdd_month(get<end_date>(c)),
                                             yyyymmdd_day(get<end_date>(c))}}));
  }
  return services;
}

}  // namespace gtfs
}  // namespace loader
}  // namespace motis
