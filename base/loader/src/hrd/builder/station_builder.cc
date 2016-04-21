#include "motis/loader/hrd/builder/station_builder.h"

#include "motis/core/common/get_or_create.h"
#include "motis/loader/hrd/files.h"
#include "motis/loader/util.h"

namespace motis {
namespace loader {
namespace hrd {

using namespace flatbuffers;

station_builder::station_builder(
    std::map<int, intermediate_station> hrd_stations, timezones tz)
    : hrd_stations_(std::move(hrd_stations)), timezones_(std::move(tz)){};

Offset<Station> station_builder::get_or_create_station(int eva_num,
                                                       FlatBufferBuilder& fbb) {
  return get_or_create(fbs_stations_, eva_num, [&]() {
    auto it = hrd_stations_.find(eva_num);
    verify(it != end(hrd_stations_), "missing station: %d", eva_num);
    auto tze = timezones_.find(eva_num);
    return CreateStation(
        fbb, to_fbs_string(fbb, pad_to_7_digits(eva_num)),
        to_fbs_string(fbb, it->second.name_, ENCODING), it->second.lat_,
        it->second.lng_, it->second.change_time_,
        fbb.CreateVector(transform_to_vec(
            begin(it->second.ds100_), end(it->second.ds100_),
            [&](std::string const& s) { return fbb.CreateString(s); })),
        get_or_create(fbs_timezones_, tze, [&]() {
          if (tze->season_) {
            auto const& season = *(tze->season_);
            return CreateTimezone(
                fbb, tze->general_gmt_offset_,
                CreateSeason(fbb, season.gmt_offset_, season.first_day_idx_,
                             season.last_day_idx_, season.season_begin_time_,
                             season.season_end_time_));
          } else {
            return CreateTimezone(fbb, tze->general_gmt_offset_);
          }
        }));
  });
}

}  // namespace hrd
}  // namespace loader
}  // namespace motis
