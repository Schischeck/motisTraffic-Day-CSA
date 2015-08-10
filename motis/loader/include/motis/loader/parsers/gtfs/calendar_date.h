#pragma once

#include <tuple>

namespace motis {
namespace loader {
namespace gtfs {

using calendar_date = std::tuple<int,  // service_id,
                                 int,  // date,
                                 int  // exception_type
                                 >;

static const std::array<parser::cstr, std::tuple_size<calendar_date>::value>
    calendar_date_columns = {{"service_id", "date", "exception_type"}};

namespace calendar_date_accessors {
enum { service_id, date, exception_type };
}  // namespace calendar_date_accessor

}  // namespace gtfs
}  // namespace loader
}  // namespace motis
