#include "motis/railviz/date_converter.h"


namespace motis {
namespace railviz {

date_converter::date_converter(): date_manager_(0) {}

date_converter::date_converter(const motis::date_manager &mgr) :
    date_manager_(&mgr) {}

void date_converter::set_date_manager( const motis::date_manager &mgr )
{
    date_manager_ = &mgr;
}

std::time_t date_converter::convert(const time &t) const
{
    if(date_manager_ == 0)
        return 0;
    return convert_to_unix_time(t);

    unsigned int day = t / MINUTES_A_DAY;
    unsigned int seconds = (t%MINUTES_A_DAY)*60;
    motis::date_manager::date const& date = date_manager_->get_date(day);
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

std::time_t date_converter::convert_to_unix_time(const motis::time& td_time) const {
    // Be aware TD works with a local time (Europe/Berlin)
    int td_day_index = td_time / MINUTES_A_DAY;
    // chach borders
    if (td_day_index > date_manager_->get_day_index(date_manager_->last_date())
            || td_day_index < date_manager_->get_day_index(date_manager_->first_date())) {
        return 0;
    }
    motis::date_manager::date const& td_date = date_manager_->get_date(td_day_index);
    std::tm local_time_struct;
    local_time_struct.tm_hour = 0;
    local_time_struct.tm_min = td_time % MINUTES_A_DAY;
    local_time_struct.tm_sec = 0;
    local_time_struct.tm_year = td_date.year - 1900;
    local_time_struct.tm_mon = td_date.month - 1;
    local_time_struct.tm_mday = td_date.day;
    // bacause TD uses Localtime, we are using std::mktime: local_time -> unixtime
    std::time_t unix_time = std::mktime(&local_time_struct);
    return unix_time;
}

std::time_t date_converter::get_unix_timestamp() {
    std::chrono::seconds sec = std::chrono::duration_cast<std::chrono::seconds> (
                std::chrono::system_clock::now().time_since_epoch()
                );
    return sec.count();
}
}
}
