#include "motis/railviz/date_converter.h"


namespace motis {
namespace railviz {

date_converter::date_converter(const motis::date_manager &mgr) :
    mgr(mgr) {}

std::time_t date_converter::convert(const time &t) const
{
    unsigned int day = t / MINUTES_A_DAY;
    unsigned int seconds = (t%MINUTES_A_DAY)*60;
    motis::date_manager::date const& date = mgr.get_date(day);
    std::time_t unix_basetime = convert(date);
    return unix_basetime + seconds;
}

std::time_t date_converter::convert(const motis::date_manager::date& d) const
{
    std::tm time_str;
    time_str.tm_hour = 0;
    time_str.tm_min = 0;
    time_str.tm_sec = 0;
    time_str.tm_year = d.year-1900;
    time_str.tm_mon = d.month-1;
    time_str.tm_mday = d.day;
    return std::mktime(&time_str);
}

}
}
