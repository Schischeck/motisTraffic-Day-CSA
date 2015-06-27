#include "motis/railviz/date_converter.h"


namespace motis {
namespace railviz {

date_converter::date_converter(const motis::date_manager &mgr) :
    mgr(mgr) {}

std::time_t date_converter::convert(const time &t) const
{
    unsigned int day = t / MINUTES_A_DAY;
    unsigned int mseconds = (t%MINUTES_A_DAY)*60*1000;
    motis::date_manager::date const& date = mgr.get_date(mgr.convert(day));
    std::tm time_str;
    time_str.tm_hour = 0;
    time_str.tm_min = 0;
    time_str.tm_sec = 0;
    time_str.tm_year = date.year;
    time_str.tm_mon = date.month-1;
    time_str.tm_mday = date.day;
    std::time_t unix_basetime = std::mktime(&time_str);
    return unix_basetime + mseconds;
}

}
}
