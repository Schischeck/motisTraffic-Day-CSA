#include "motis/routes/db/db_builder.h"

#include "motis/core/common/transform_to_vec.h"

namespace motis {
namespace routes {

int db_builder::append(std::vector<std::string> const& station_ids,
                       std::vector<uint32_t> const& classes,
                       std::vector<std::vector<geo::latlng>> const& lines) {
  std::lock_guard<std::mutex> lock(m_);
  module::message_creator b;
  auto fbs_stations = transform_to_vec(
      station_ids,
      [&](auto const& station_id) { return b.CreateString(station_id); });

  std::vector<flatbuffers::Offset<Polyline>> fbs_lines;
  for (auto const& line : lines) {
    std::vector<double> flat_polyline;
    for (auto const& latlng : line) {
      flat_polyline.push_back(latlng.lat_);
      flat_polyline.push_back(latlng.lng_);
    }
    fbs_lines.push_back(CreatePolyline(b, b.CreateVector(flat_polyline)));
  }

  auto res = CreateRoutesSeqResponse(b, b.CreateVector(fbs_stations),
                                     b.CreateVector(classes),
                                     b.CreateVector(fbs_lines));
  b.create_and_finish(MsgContent_RoutesSeqResponse, res.Union());

  db_.put(std::to_string(index_), routing_sequence(std::move(b)).to_string());
  indices_.emplace_back(station_ids, classes);
  index_++;
  return index_;
}

void db_builder::finish() {
  module::message_creator b;
  std::vector<flatbuffers::Offset<motis::routes::RouteIndex>> r;
  std::sort(begin(indices_), end(indices_), [&](auto const& l, auto const& r) {
    return std::tie(l.first, l.second) < std::tie(r.first, r.second);
  });
  for (auto i = 0u; i < indices_.size(); i++) {
    auto ids = indices_[i].first;
    auto classes = indices_[i].second;

    auto fbs_stations = transform_to_vec(ids, [&](auto const& station_id) {
      return b.CreateString(station_id);
    });

    r.push_back(CreateRouteIndex(b, b.CreateVector(fbs_stations),
                                 b.CreateVector(classes), i));
  }

  b.Finish(CreateRouteLookup(b, b.CreateVector(r)));
  db_.put("__index", routing_lookup(std::move(b)).to_string());
}

}  // namespace routes
}  // namespace motis
