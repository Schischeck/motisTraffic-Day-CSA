#pragma once

#include <tuple>

#include "parser/cstr.h"

namespace motis {
namespace loader {
namespace gtfs {

using route = std::tuple<int,  // route_id
                         parser::cstr,  // agency_id
                         parser::cstr,  // route_short_name
                         parser::cstr,  // route_long_name
                         parser::cstr,  // route_desc
                         int,  // route_type
                         parser::cstr,  // route_url
                         int,  // route_color
                         int  // route_text_color
                         >;

static const std::array<parser::cstr, std::tuple_size<route>::value> route_columns = {
    {"route_id", "agency_id", "route_short_name", "route_long_name",
     "route_desc", "route_type", "route_url", "route_color",
     "route_text_color"}};

namespace route_accessors {
enum {
  route_id,
  agency_id,
  route_short_name,
  route_long_name,
  route_desc,
  route_type,
  route_url,
  route_color,
  route_text_color
};
}  // namespace route_accessors

}  // namespace gtfs
}  // namespace loader
}  // namespace motis
