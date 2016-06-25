#include "motis/routes/routes_section.h"

#include "flatbuffers/flatbuffers.h"

#include "motis/protocol/RoutesSections_generated.h"

namespace motis {
namespace routes {

flatbuffers::Offset<RoutesSectionRes> routes_railroad_sec_with_data(
    station const* departure, station const* arrival, int& clasz,
    motis::module::message_creator& b, parser::buffer& buf_) {
  auto railroad_sections = GetRoutesSections(buf_.buf_);
  auto it = std::lower_bound(
      railroad_sections->sections()->begin(),
      railroad_sections->sections()->end(),
      std::make_pair(departure->index_, arrival->index_),
      [](RoutesSection const* a, std::pair<int, int> const& b) {
        return std::make_tuple(a->from(), a->to()) <
               std::tie(b.first, b.second);
      });
  if (it != railroad_sections->sections()->end() &&
      it->from() == departure->index_ && it->to() == arrival->index_) {
    std::vector<double> section;
    auto best = it;
    while (it != railroad_sections->sections()->end() && it->clasz() != clasz &&
           (it->from() == departure->index_ && it->to() == arrival->index_)) {
      ++it;
      if (it->clasz() < best->clasz()) {
        best = it;
      }
    }
    for (auto const d : *best->section()) {
      section.push_back(d);
    }
    return CreateRoutesSectionRes(b, b.CreateVector(section), best->clasz());
  } else {
    return routes_railroad_sec_without_data(departure, arrival, clasz, b);
  }
}

flatbuffers::Offset<RoutesSectionRes> routes_railroad_sec_without_data(
    station const* departure, station const* arrival, int& clasz,
    motis::module::message_creator& b) {
  std::vector<double> section;
  section.push_back(departure->lat());
  section.push_back(departure->lng());
  section.push_back(arrival->lat());
  section.push_back(arrival->lng());
  return CreateRoutesSectionRes(b, b.CreateVector(section), clasz);
}

}  // namespace routes
}  // namespace motis
