#pragma once

#include <cinttypes>
#include <vector>

#include "motis/schedule-format/Service_generated.h"

#include "motis/loader/hrd/model/hrd_service.h"
#include "motis/loader/hrd/parser/platform_rules_parser.h"
#include "motis/loader/hrd/builder/route_builder.h"
#include "motis/loader/hrd/builder/station_builder.h"
#include "motis/loader/hrd/builder/category_builder.h"
#include "motis/loader/hrd/builder/provider_builder.h"
#include "motis/loader/hrd/builder/line_builder.h"
#include "motis/loader/hrd/builder/attribute_builder.h"
#include "motis/loader/hrd/builder/bitfield_builder.h"
#include "motis/loader/hrd/builder/direction_builder.h"

namespace motis {
namespace loader {
namespace hrd {

struct service_builder {
  service_builder(platform_rules);

  void create_service(hrd_service const&, route_builder&, station_builder&,
                      category_builder&, provider_builder&, line_builder&,
                      attribute_builder&, bitfield_builder&, direction_builder&,
                      flatbuffers::FlatBufferBuilder&);

  platform_rules const plf_rules_;
  std::vector<flatbuffers::Offset<Service>> fbs_services_;
};

}  // hrd
}  // loader
}  // motis
