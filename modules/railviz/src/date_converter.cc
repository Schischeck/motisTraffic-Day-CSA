#include "motis/railviz/date_converter.h"

#include <cstring>

namespace motis {
namespace railviz {

date_converter::date_converter() : date_manager_(0) {}

date_converter::date_converter(const date_manager& mgr) : date_manager_(&mgr) {}

void date_converter::set_date_manager(const date_manager& mgr) {
  date_manager_ = &mgr;
}

std::time_t date_converter::convert(const time& t) const {
  return convert_to_unix_time(t);
}

std::time_t date_converter::convert(const date_manager::date& d) const {
  std::tm time_str;
  std::memset(&time_str, 0, sizeof(time_str));
  time_str.tm_year = d.year - 1900;
  time_str.tm_mon = d.month - 1;
  time_str.tm_mday = d.day;
  return std::mktime(&time_str);
}

time date_converter::convert_to_motis(std::time_t t) const {
  auto first_time_in_timetable = convert(date_manager_->first_date());
  return (t - first_time_in_timetable) / 60;
}

time date_converter::convert_to_motis(const date_manager::date d) const {
  std::time_t time_of_date = convert(d);
  std::time_t first_time_in_timetable = convert(date_manager_->first_date());
  return (time_of_date - first_time_in_timetable) / 60;
}

std::time_t date_converter::convert_to_unix_time(const time& td_time) const {
  // Be aware TD works with a local time (Europe/Berlin)
  int day_index = td_time / MINUTES_A_DAY;

  // Catch borders.
  if (day_index > date_manager_->get_day_index(date_manager_->last_date()) ||
      day_index < date_manager_->get_day_index(date_manager_->first_date())) {
    return 0;
  }

  date_manager::date const& td_date = date_manager_->get_date(day_index);
  std::tm local_time_struct;
  std::memset(&local_time_struct, 0, sizeof(local_time_struct));
  local_time_struct.tm_min = td_time % MINUTES_A_DAY;
  local_time_struct.tm_year = td_date.year - 1900;
  local_time_struct.tm_mon = td_date.month - 1;
  local_time_struct.tm_mday = td_date.day;

  // bacause TD uses Localtime, we are using std::mktime: local_time -> unixtime
  return std::mktime(&local_time_struct);
}

}  // namespace railviz
}  // namespace motis
