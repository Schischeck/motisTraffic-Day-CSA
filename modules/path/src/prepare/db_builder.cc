#include "motis/path/prepare/db_builder.h"

#include "utl/to_vec.h"

using namespace flatbuffers;
using namespace motis::module;

namespace motis {
namespace path {

constexpr char kIndexKey[] = "__index";

int db_builder::append(std::vector<std::string> const& station_ids,
                       std::vector<uint32_t> const& classes,
                       std::vector<std::vector<geo::latlng>> const& lines,
                       std::vector<sequence_info> const& sequence_infos) {
  std::lock_guard<std::mutex> lock(m_);

  message_creator b;
  auto const fbs_stations = utl::to_vec(
      station_ids, [&](auto const& id) { return b.CreateString(id); });

  std::vector<Offset<Polyline>> fbs_lines;
  for (auto const& line : lines) {
    std::vector<double> flat_polyline;
    for (auto const& latlng : line) {
      flat_polyline.push_back(latlng.lat_);
      flat_polyline.push_back(latlng.lng_);
    }
    fbs_lines.push_back(CreatePolyline(b, b.CreateVector(flat_polyline)));
  }

  std::vector<Offset<PathSourceInfo>> fbs_info;
  for (auto const& info : sequence_infos) {
    fbs_info.push_back(CreatePathSourceInfo(b, info.idx_, info.from_, info.to_,
                                            b.CreateString(info.type_)));
  }

  auto res = CreatePathSeqResponse(
      b, b.CreateVector(fbs_stations), b.CreateVector(classes),
      b.CreateVector(fbs_lines), b.CreateVector(fbs_info));
  b.create_and_finish(MsgContent_PathSeqResponse, res.Union());

  db_->put(std::to_string(index_), routing_sequence(std::move(b)).to_string());
  indices_.emplace_back(station_ids, classes, index_);
  index_++;
  return index_;
}

void db_builder::finish() {
  message_creator b;
  std::vector<Offset<motis::path::PathIndex>> r;
  std::sort(begin(indices_), end(indices_));

  for (auto i = 0u; i < indices_.size(); ++i) {
    auto ids = std::get<0>(indices_[i]);
    auto classes = std::get<1>(indices_[i]);

    auto const fbs_stations =
        utl::to_vec(ids, [&](auto const& id) { return b.CreateString(id); });

    r.push_back(CreatePathIndex(b, b.CreateVector(fbs_stations),
                                b.CreateVector(classes),
                                std::get<2>(indices_[i])));
  }

  b.Finish(CreatePathLookup(b, b.CreateVector(r)));
  db_->put(kIndexKey, routing_lookup(std::move(b)).to_string());
}

}  // namespace path
}  // namespace motis
