#pragma once

#include <cinttypes>
#include <map>
#include <vector>

#include "parser/cstr.h"

#include "motis/schedule-format/Schedule_generated.h"

#include "motis/loader/model/hrd/shared_data.h"
#include "motis/loader/model/hrd/hrd_service.h"
#include "motis/loader/builders/hrd/bitfield_builder.h"
#include "motis/loader/builders/hrd/provider_builder.h"
#include "motis/loader/builders/hrd/station_builder.h"
#include "motis/loader/builders/hrd/rule_service_builder.h"

namespace motis {
namespace loader {
namespace hrd {

struct service_builder {
  service_builder(attribute_builder&, bitfield_builder&, category_builder&,
                  direction_builder&, line_builder&, provider_builder&,
                  station_builder&);

  void build_service(hrd_service const&, flatbuffers::FlatBufferBuilder&);

  flatbuffers::Offset<Route> create_route(
      std::vector<hrd_service::stop> const&);

  flatbuffers::Offset<flatbuffers::Vector<flatbuffers::Offset<Section>>>
  create_sections(std::vector<hrd_service::section> const&);

  flatbuffers::Offset<flatbuffers::Vector<flatbuffers::Offset<PlatformRules>>>
  create_platforms(std::vector<hrd_service::section> const&,
                   std::vector<hrd_service::stop> const&);

  void resolve_platform_rule(platform_rule_key const& key, int time,
                             std::vector<flatbuffers::Offset<Platform>>&);

  flatbuffers::Offset<flatbuffers::Vector<int32_t>> create_times(
      std::vector<hrd_service::stop> const&);

  typedef std::tuple<int, bool, bool> station_events;
  std::map<std::vector<station_events>, flatbuffers::Offset<Route>> routes_;
  std::vector<flatbuffers::Offset<Service>> services_;

  attribute_builder& ab_;
  bitfield_builder& bb_;
  category_builder& cb_;
  direction_builder& db_;
  line_builder& lb_;
  provider_builder& pb_;
  station_builder& sb_;
};

}  // hrd
}  // loader
}  // motis
