#include "motis/loader/parsers/gtfs/calendar_date.h"

#include "parser/csv.h"

#include "motis/loader/util.h"

using namespace parser;
using std::get;

namespace motis {
namespace loader {
namespace gtfs {

using gtfs_calendar_date = std::tuple<cstr, int, int>;
enum { service_id, date_column, exception_type };

static const column_mapping<gtfs_calendar_date> calendar_columns = {
    {"service_id", "date", "exception_type"}};

date read_date(gtfs_calendar_date const& gtfs_date) {
  date d;
  d.day = {yyyymmdd_year(get<date_column>(gtfs_date)),
           yyyymmdd_month(get<date_column>(gtfs_date)),
           yyyymmdd_day(get<date_column>(gtfs_date))};
  d.type = get<exception_type>(gtfs_date) == 1 ? date::ADD : date::REMOVE;
  return d;
}

std::map<std::string, std::vector<date>> read_calendar_date(loaded_file f) {
  std::map<std::string, std::vector<date>> services;
  for (auto const& d : read<gtfs_calendar_date>(f.content, calendar_columns)) {
    services[get<service_id>(d).to_str()].push_back(read_date(d));
  }
  return services;
}

}  // namespace gtfs
}  // namespace loader
}  // namespace motis
