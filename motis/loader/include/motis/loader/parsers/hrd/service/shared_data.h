#pragma once

#include <cinttypes>
#include <string>
#include <vector>
#include <map>

#include "flatbuffers/flatbuffers.h"

#include "motis/schedule-format/Interval_generated.h"

#include "motis/loader/parsers/hrd/station_meta_data_parser.h"
#include "motis/loader/parsers/hrd/stations_parser.h"
#include "motis/loader/parsers/hrd/categories_parser.h"
#include "motis/loader/parsers/hrd/bitfields_parser.h"
#include "motis/loader/parsers/hrd/platform_rules_parser.h"
#include "motis/loader/parsers/hrd/providers_parser.h"
#include "motis/loader/parsers/hrd/through_services_parser.h"
#include "motis/loader/parsers/hrd/merge_split_rules_parser.h"

namespace motis {
namespace loader {
namespace hrd {

struct shared_data {
  shared_data() = default;
  shared_data(Interval interval, station_meta_data metas,
              std::map<int, intermediate_station> stations,
              std::map<uint32_t, category> categories,
              std::map<uint16_t, std::string> attributes,
              std::map<int, bitfield> bitfields, platform_rules pf_rules,
              std::map<uint64_t, std::string> directions,
              std::map<uint64_t, provider_info> providers,
              std::vector<through_service_rule> through_service_rules,
              std::vector<merge_split_service_rule> merge_split_rules)
      : interval(std::move(interval)),
        metas(std::move(metas)),
        stations(std::move(stations)),
        categories(std::move(categories)),
        attributes(std::move(attributes)),
        bitfields(std::move(bitfields)),
        pf_rules(std::move(pf_rules)),
        directions(std::move(directions)),
        providers(std::move(providers)),
        through_service_rules(std::move(through_service_rules)),
        merge_split_rules(std::move(merge_split_rules)) {}

  Interval interval;
  station_meta_data metas;
  std::map<int, intermediate_station> stations;
  std::map<uint32_t, category> categories;
  std::map<uint16_t, std::string> attributes;
  std::map<int, bitfield> bitfields;
  platform_rules pf_rules;
  std::map<uint64_t, std::string> directions;
  std::map<uint64_t, provider_info> providers;
  std::vector<through_service_rule> through_service_rules;
  std::vector<merge_split_service_rule> merge_split_rules;
};

}  // namespace hrd
}  // namespace loader
}  // namespace motis
