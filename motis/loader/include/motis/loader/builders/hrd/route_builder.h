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

struct stop_restrictions {
  friend bool operator<(stop_restrictions const& rhs,
                        stop_restrictions const& lhs) {
    if (rhs.eva_num < lhs.eva_num) {
      return true;
    }
    if (!rhs.entering_allowed && lhs.entering_allowed) {
      return true;
    }
    if (!rhs.leaving_allowed && lhs.leaving_allowed) {
      return true;
    }
    return false;
  }

  int eva_num;
  bool entering_allowed;
  bool leaving_allowed;
};

struct route_builder {
  flatbuffers::Offset<Route> get_or_create_route(
      std::vector<hrd_service::stop> const&, station_builder& sb_,
      flatbuffers::FlatBufferBuilder&);

  std::map<std::vector<stop_restrictions>, flatbuffers::Offset<Route>> routes_;
};

}  // hrd
}  // loader
}  // motis
