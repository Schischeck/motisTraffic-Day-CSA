#pragma once

#include <cinttypes>
#include <map>
#include <vector>

#include "parser/cstr.h"

#include "motis/loader/parsers/hrd/bitfield_translator.h"
#include "motis/loader/parsers/hrd/stations_translator.h"
#include "motis/loader/parsers/hrd/providers_translator.h"
#include "motis/loader/parsers/hrd/service/shared_data.h"
#include "motis/loader/parsers/hrd/service/hrd_service.h"
#include "motis/schedule-format/Schedule_generated.h"

namespace motis {
namespace loader {
namespace hrd {

struct service_builder {
  typedef std::tuple<int, bool, bool> station_events;

  service_builder(shared_data const& stamm,
                  flatbuffers::FlatBufferBuilder& builder);

  void create_services(hrd_service&&);

  flatbuffers::Offset<Route> create_route(
      std::vector<hrd_service::stop> const&);

  flatbuffers::Offset<flatbuffers::Vector<flatbuffers::Offset<Section>>>
  create_sections(std::vector<hrd_service::section> const&);

  flatbuffers::Offset<Category> get_or_create_category(
      parser::cstr category_code);

  flatbuffers::Offset<Attribute> get_or_create_attribute(
      hrd_service::attribute attr);

  flatbuffers::Offset<flatbuffers::Vector<flatbuffers::Offset<Attribute>>>
  create_attributes(std::vector<hrd_service::attribute> const&);

  flatbuffers::Offset<flatbuffers::String> get_or_create_line_info(
      std::vector<parser::cstr> const& line_info);

  flatbuffers::Offset<Direction> get_or_create_direction(
      std::vector<std::pair<uint64_t, int>> const& direction_key);

  flatbuffers::Offset<flatbuffers::Vector<flatbuffers::Offset<PlatformRules>>>
  create_platforms(std::vector<hrd_service::section> const&,
                   std::vector<hrd_service::stop> const&);

  void resolve_platform_rule(platform_rule_key const& key, int time,
                             std::vector<flatbuffers::Offset<Platform>>&);

  flatbuffers::Offset<flatbuffers::Vector<int32_t>> create_times(
      std::vector<hrd_service::stop> const&);

  shared_data const& stamm_;
  bitfield_translator bitfields_;
  stations_translator stations_;
  providers_translator providers_;
  flatbuffers::FlatBufferBuilder& builder_;
  std::vector<flatbuffers::Offset<Service>> services_;
  std::map<uint16_t, flatbuffers::Offset<AttributeInfo>> attribute_infos_;
  std::map<std::pair<uint16_t, int>, flatbuffers::Offset<Attribute>>
      attributes_;
  std::map<uint32_t, flatbuffers::Offset<Category>> categories_;
  std::map<uint64_t, flatbuffers::Offset<flatbuffers::String>> line_infos_;
  std::map<uint64_t, flatbuffers::Offset<Direction>> directions_;
  std::map<std::vector<station_events>, flatbuffers::Offset<Route>> routes_;
};

}  // hrd
}  // loader
}  // motis
