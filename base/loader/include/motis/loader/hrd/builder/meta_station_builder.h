#pragma once

#include "motis/loader/hrd/builder/station_builder.h"
#include "motis/loader/hrd/parser/station_meta_data_parser.h"

#include "motis/schedule-format/MetaStation_generated.h"

#include <map>
#include <vector>

namespace motis {
namespace loader {
namespace hrd {

flatbuffers64::Offset<flatbuffers64::Vector<flatbuffers64::Offset<MetaStation>>>
create_meta_stations(std::set<station_meta_data::meta_station> const&,
                     std::map<int, flatbuffers64::Offset<Station>> const&,
                     flatbuffers64::FlatBufferBuilder&);

flatbuffers64::Offset<flatbuffers64::Vector<flatbuffers64::Offset<MetaStation>>>
create_meta_stations(std::set<station_meta_data::meta_station> const&,
                     station_builder& sb, flatbuffers64::FlatBufferBuilder&);

void add_equivalent_stations(std::vector<int>&, int,
                             std::set<station_meta_data::meta_station> const&);

std::vector<int> get_equivalent_stations(
    station_meta_data::meta_station const&,
    std::set<station_meta_data::meta_station> const&);

}  // namespace hrd
}  // namespace loader
}  // namespace motis
