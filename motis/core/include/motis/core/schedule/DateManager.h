#ifndef TDDATEMANAGER_H
#define TDDATEMANAGER_H TDDATEMANAGER_H

#include <map>
#include <vector>
#include <sstream>
#include <iomanip>
#include <iostream>

#include "motis/core/schedule/Time.h"

namespace td
{

/** Hides away all indexing details of dates. It's main task is to convert
 *  dates into day-indices */
class DateManager
{
  public:
    /** Simple representation of a date */
    struct Date
    {
      Date(int d, int m, int y) : day(d), month(m), year(y)
      {}

      Date() : day(0), month(0), year(0)
      {}

      std::istream& operator>>(std::istream& in)
      {
        char c;
        in >> day >> c >> month >> c >> year;
        return in;
      }

      bool operator<(const Date& o) const
      {return day + 32*month + 32*13*year < o.day + 32*o.month + 32*13*o.year;}

      std::string str() const
      {
        std::stringstream s;
        s << std::setw(2) << std::setfill('0') << day << '.'
          << std::setw(2) << std::setfill('0') << month << '.'
          << year;
        return s.str();
      }

      int day, month, year;
    };

    std::string formatISO(Time time) const
    {
      if (time == INVALID_TIME)
        return "INVALID";

      int day = time / MINUTES_A_DAY;
      int minutes = time % MINUTES_A_DAY;

      auto date = getDate(day);
      std::ostringstream out;
      out << std::setw(4) << std::setfill('0') << date.year << "-"
          << std::setw(2) << std::setfill('0') << date.month << "-"
          << std::setw(2) << std::setfill('0') << date.day << "T"
          << std::setw(2) << std::setfill('0') << (minutes / 60) << ":"
          << std::setw(2) << std::setfill('0') << (minutes % 60);

      return out.str();
    }

    /** As the indices are stored kind of weird (°_°) we reverse them and add 1
     *  so that they are 1 based.
     *  @param vector with all the dates (position in vector = index) */
    void load(const std::vector<Date> dates)
    {
      _firstDate = dates[dates.size() - 1];
      for(int i = dates.size() - 1; i >= 0; --i)
      {
        auto index = dates.size() - i - 1;
        _indices[dates[i]] = index;
        _indexToDateMap[index] = dates[i];
      }
      _lastDate = dates[0];
    }

    /** @return the dayIndex of the given date (1-based) */
    int getDayIndex(const Date& date) const
    {
      std::map<Date, int>::const_iterator it = _indices.find(date);
      if(it == _indices.end())
        return NO_INDEX;
      else
        return it->second;
    }

    const Date& getDate(int index) const
    { return _indexToDateMap.at(index); }

    /** @return converted date (1 based and starting at the beginning of the
     *          time interval. Motis stores dates totally confusing :-) */
    int convert(int day) const
    { return _indices.size() - day; }

    const Date& firstDate() const { return _firstDate; }
    const Date& lastDate() const { return _lastDate; }

    static const int NO_INDEX = -1;

  private:
    std::map<Date, int /* date index */> _indices;
    std::map<int /* date index */, Date> _indexToDateMap;
    Date _firstDate, _lastDate;
};

}

#endif //TDDATEMANAGER_H

