#pragma once

#include <map>
#include <vector>
#include <sstream>
#include <iomanip>
#include <iostream>

#include "motis/core/schedule/time.h"

namespace td {

/** hides away all indexing details of dates. it's main task is to convert
 *  dates into day-indices */
class date_manager {
public:
  /** simple representation of a date */
  struct date {
    date(int d, int m, int y) : day(d), month(m), year(y) {}

    date() : day(0), month(0), year(0) {}

    std::istream& operator>>(std::istream& in) {
      char c;
      in >> day >> c >> month >> c >> year;
      return in;
    }

    bool operator<(const date& o) const {
      return day + 32 * month + 32 * 13 * year <
             o.day + 32 * o.month + 32 * 13 * o.year;
    }

    std::string str() const {
      std::stringstream s;
      s << std::setw(2) << std::setfill('0') << day << '.' << std::setw(2)
        << std::setfill('0') << month << '.' << year;
      return s.str();
    }

    int day, month, year;
  };

  std::string format_i_s_o(time time) const {
    if (time == INVALID_TIME) return "INVALID";

    int day = time / MINUTES_A_DAY;
    int minutes = time % MINUTES_A_DAY;

    auto date = get_date(day);
    std::ostringstream out;
    out << std::setw(4) << std::setfill('0') << date.year << "-" << std::setw(2)
        << std::setfill('0') << date.month << "-" << std::setw(2)
        << std::setfill('0') << date.day << "t" << std::setw(2)
        << std::setfill('0') << (minutes / 60) << ":" << std::setw(2)
        << std::setfill('0') << (minutes % 60);

    return out.str();
  }

  /** as the indices are stored kind of weird (°_°) we reverse them and add 1
   *  so that they are 1 based.
   *  @param vector with all the dates (position in vector = index) */
  void load(const std::vector<date> dates) {
    _first_date = dates[dates.size() - 1];
    for (int i = dates.size() - 1; i >= 0; --i) {
      auto index = dates.size() - i - 1;
      _indices[dates[i]] = index;
      _index_to_date_map[index] = dates[i];
    }
    _last_date = dates[0];
  }

  /** @return the day_index of the given date (1-based) */
  int get_day_index(const date& date) const {
    auto it = _indices.find(date);
    if (it == _indices.end())
      return NO_INDEX;
    else
      return it->second;
  }

  const date& get_date(int index) const { return _index_to_date_map.at(index); }

  /** @return converted date (1 based and starting at the beginning of the
   *          time interval. motis stores dates totally confusing :-) */
  int convert(int day) const { return _indices.size() - day; }

  const date& first_date() const { return _first_date; }
  const date& last_date() const { return _last_date; }

  static const int NO_INDEX = -1;

private:
  std::map<date, int /* date index */> _indices;
  std::map<int /* date index */, date> _index_to_date_map;
  date _first_date, _last_date;
};

}  // namespace td
