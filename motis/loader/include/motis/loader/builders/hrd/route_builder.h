#pragma once

#include <cinttypes>
#include <map>
#include <vector>

#include "parser/cstr.h"

#include "motis/schedule-format/Schedule_generated.h"

#include "motis/loader/model/hrd/hrd_service.h"
#include "motis/loader/builders/hrd/bitfield_builder.h"
#include "motis/loader/builders/hrd/provider_builder.h"
#include "motis/loader/builders/hrd/station_builder.h"
#include "motis/loader/builders/hrd/rule_service_builder.h"

namespace motis {
namespace loader {
namespace hrd {

struct stop_restrictions {
  friend bool operator<(stop_restrictions const& lhs,
                        stop_restrictions const& rhs) {
    if (rhs.eva_num < lhs.eva_num) {
      return true;
    } else if (rhs.eva_num > lhs.eva_num) {
      return false;
    }
    if (rhs.entering_allowed < lhs.entering_allowed) {
      return true;
    } else if (rhs.entering_allowed > lhs.entering_allowed) {
      return false;
    }
    if (rhs.leaving_allowed < lhs.leaving_allowed) {
      return true;
    } else if (rhs.entering_allowed > lhs.entering_allowed) {
      return false;
    }
    return false;
  }

  friend bool operator==(stop_restrictions const& lhs,
                         stop_restrictions const& rhs) {
    return lhs.eva_num == rhs.eva_num &&
           lhs.entering_allowed == rhs.entering_allowed &&
           lhs.leaving_allowed == rhs.leaving_allowed;
  }

  int eva_num;
  bool entering_allowed;
  bool leaving_allowed;
};

struct route_builder {
  flatbuffers::Offset<Route> get_or_create_route(
      std::vector<hrd_service::stop> const&, station_builder&,
      flatbuffers::FlatBufferBuilder&);

  std::map<std::vector<stop_restrictions>, flatbuffers::Offset<Route>> routes_;
};

}  // hrd
}  // loader
}  // motis
