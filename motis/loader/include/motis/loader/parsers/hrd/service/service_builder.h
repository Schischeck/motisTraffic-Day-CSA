#pragma once

#include <cinttypes>
#include <map>
#include <vector>

#include "parser/cstr.h"

#include "motis/loader/parsers/hrd/bitfield_translator.h"
#include "motis/loader/parsers/hrd/service/shared_data.h"
#include "motis/loader/parsers/hrd/service/hrd_service.h"
#include "motis/schedule-format/Schedule_generated.h"

namespace motis {
namespace loader {
namespace hrd {

struct service_builder {
  service_builder(shared_data const& stamm,
                  flatbuffers::FlatBufferBuilder& builder);

  flatbuffers::Offset<flatbuffers::String> get_or_create_category(
      parser::cstr category);

  flatbuffers::Offset<flatbuffers::String> get_or_create_line_info(
      parser::cstr line_info);

  flatbuffers::Offset<Route> create_route(
      std::vector<hrd_service::stop> const&);

  flatbuffers::Offset<Attribute> get_or_create_attribute(
      hrd_service::attribute attr);

  flatbuffers::Offset<flatbuffers::Vector<flatbuffers::Offset<Attribute>>>
  create_attributes(std::vector<hrd_service::attribute> const&);

  flatbuffers::Offset<flatbuffers::Vector<flatbuffers::Offset<Section>>>
  create_sections(std::vector<hrd_service::section> const&);

  void resolve_platform_rule(platform_rule_key const& key, int time,
                             std::vector<flatbuffers::Offset<Platform>>&);

  flatbuffers::Offset<flatbuffers::Vector<flatbuffers::Offset<PlatformRules>>>
  create_platforms(std::vector<hrd_service::section> const&,
                   std::vector<hrd_service::stop> const&);

  flatbuffers::Offset<flatbuffers::Vector<uint64_t>> create_times(
      std::vector<hrd_service::stop> const&);

  void create_services(hrd_service const&, flatbuffers::FlatBufferBuilder&,
                       std::vector<flatbuffers::Offset<Service>>&);

  shared_data const& stamm_;
  std::map<uint16_t, flatbuffers::Offset<Attribute>> attributes_;
  std::map<uint32_t, flatbuffers::Offset<flatbuffers::String>> categories_;
  std::map<uint64_t, flatbuffers::Offset<flatbuffers::String>> line_infos_;
  std::map<std::vector<int>, flatbuffers::Offset<Route>> routes_;
  bitfield_translator bitfields_;
  flatbuffers::FlatBufferBuilder& builder_;
};

}  // hrd
}  // loader
}  // motis
