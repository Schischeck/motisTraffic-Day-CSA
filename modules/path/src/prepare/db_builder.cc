#include "motis/path/prepare/db_builder.h"

#include "utl/get_or_create.h"
#include "utl/to_vec.h"

#include "parser/util.h"

#include "motis/path/constants.h"

using namespace flatbuffers;
using namespace motis::module;

namespace motis {
namespace path {

void db_builder::append(std::vector<std::string> const& station_ids,
                        std::vector<uint32_t> const& classes,
                        std::vector<std::vector<geo::latlng>> const& lines,
                        std::vector<sequence_info> const& sequence_infos) {
  std::lock_guard<std::mutex> lock(m_);
  verify(lines.size() + 1 == station_ids.size(), "db_builder: size mismatch");

  for (auto i = 0u; i < lines.size(); ++i) {
    auto key = (station_ids[i] < station_ids[i + 1])
                   ? std::make_pair(station_ids[i], station_ids[i + 1])
                   : std::make_pair(station_ids[i + 1], station_ids[i]);

    auto& box = utl::get_or_create(boxes_, key, [] {
      return std::pair<geo::latlng, geo::latlng>{
          {-std::numeric_limits<double>::infinity(),
           -std::numeric_limits<double>::infinity()},
          {std::numeric_limits<double>::infinity(),
           std::numeric_limits<double>::infinity()}};
    });

    for (auto const& pos : lines[i]) {
      box.first.lat_ = std::max(box.first.lat_, pos.lat_);
      box.first.lng_ = std::max(box.first.lng_, pos.lng_);

      box.second.lat_ = std::min(box.second.lat_, pos.lat_);
      box.second.lng_ = std::min(box.second.lng_, pos.lng_);
    }
  }

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

  b.create_and_finish(
      MsgContent_PathSeqResponse,
      CreatePathSeqResponse(
          b, b.CreateVector(fbs_stations), b.CreateVector(classes),
          b.CreateVector(fbs_lines),
          b.CreateVector(utl::to_vec(sequence_infos,
                                     [&](auto&& info) {
                                       return CreatePathSourceInfo(
                                           b, info.idx_, info.from_, info.to_,
                                           b.CreateString(info.type_));
                                     })))
          .Union());

  db_->put(std::to_string(index_), routing_sequence(std::move(b)).to_string());
  indices_.emplace_back(station_ids, classes, index_);
  ++index_;
}

void db_builder::finish() {
  finish_index();
  finish_boxes();
}

void db_builder::finish_index() {
  std::sort(begin(indices_), end(indices_));

  message_creator b;
  b.Finish(
      CreatePathLookup(b, b.CreateVector(utl::to_vec(indices_, [&](auto&& e) {
        auto const& [station_ids, classes, index] = e;
        return CreatePathIndex(
            b,
            b.CreateVector(utl::to_vec(
                station_ids,
                [&](auto const& id) { return b.CreateString(id); })),
            b.CreateVector(classes), index);
      }))));
  db_->put(kIndexKey, routing_lookup(std::move(b)).to_string());
}

void db_builder::finish_boxes() {
  message_creator mc;

  mc.create_and_finish(
      MsgContent_PathBoxesResponse,
      CreatePathBoxesResponse(
          mc, mc.CreateVector(utl::to_vec(
                  boxes_,
                  [&mc](auto const& pair) {
                    auto ne = motis::Position{pair.second.first.lat_,
                                              pair.second.first.lng_};
                    auto sw = motis::Position{pair.second.second.lat_,
                                              pair.second.second.lng_};

                    return CreateBox(
                        mc, mc.CreateSharedString(pair.first.first),
                        mc.CreateSharedString(pair.first.second), &ne, &sw);
                  })))
          .Union());

  db_->put(kBoxesKey, make_msg(mc)->to_string());
}

}  // namespace path
}  // namespace motis
