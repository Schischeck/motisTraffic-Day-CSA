#include "motis/loader/builders/hrd/service_builder.h"

#include "motis/loader/util.h"
#include "motis/loader/parsers/hrd/files.h"

using namespace parser;
using namespace flatbuffers;

namespace motis {
namespace loader {
namespace hrd {

service_builder::service_builder(attribute_builder& ab, bitfield_builder& bb,
                                 category_builder& cb, direction_builder& db,
                                 line_builder& lb, provider_builder& pb,
                                 station_builder& sb)
    : ab_(ab), bb_(bb), cb_(cb), db_(db), lb_(lb), pb_(pb), sb_(sb) {}

void service_builder::build_service(builders b, hrd_service const& s) {
  auto const route = create_route(s.stops_);
  services_.push_back(CreateService(
      fbb_, route, bb_.get_or_create_bitfield(s.traffic_days_),
      create_sections(s.sections_), create_platforms(s.sections_, s.stops_),
      create_times(s.stops_)));
}

Offset<Route> create_route(std::vector<hrd_service::stop> const& stops,
                           station_builder& sb, FlatBufferBuilder fbb) {
  auto events =
      transform_to_vec(begin(stops), end(stops),
                       [&](hrd_service::stop const& s) -> station_events {
                         return std::make_tuple(s.eva_num, s.dep.in_out_allowed,
                                                s.arr.in_out_allowed);
                       });
  return get_or_create(routes_, events, [&]() {
    return CreateRoute(
        fbb_, fbb_.CreateVector(transform_to_vec(
                  begin(events), end(events),
                  [&](station_events ev) {
                    return sb_.get_or_create_station(std::get<0>(ev), fbb_);
                  })),
        fbb_.CreateVector(transform_to_vec(begin(events), end(events),
                                           [](station_events e) -> uint8_t {
                                             return std::get<1>(e) ? 1 : 0;
                                           })),
        fbb_.CreateVector(transform_to_vec(begin(events), end(events),
                                           [](station_events e) -> uint8_t {
                                             return std::get<2>(e) ? 1 : 0;
                                           })));
  });
}

Offset<Vector<Offset<Section>>> service_builder::create_sections(
    std::vector<hrd_service::section> const& sections) {
  return fbb_.CreateVector(transform_to_vec(
      begin(sections), end(sections), [&](hrd_service::section const& s) {
        return CreateSection(
            fbb_, get_or_create_category(s.category[0]),
            pb_.get_or_create_provider(raw_to_int<uint64_t>(s.admin)),
            s.train_num, get_or_create_line_info(s.line_information),
            create_attributes(s.attributes),
            get_or_create_direction(s.directions));
      }));
}

Offset<Vector<Offset<PlatformRules>>> service_builder::create_platforms(
    std::vector<hrd_service::section> const& sections,
    std::vector<hrd_service::stop> const& stops) {
  struct stop_platforms {
    std::vector<Offset<Platform>> dep_platforms;
    std::vector<Offset<Platform>> arr_platforms;
  };

  std::vector<stop_platforms> stops_platforms(stops.size());
  for (unsigned i = 0; i < sections.size(); ++i) {
    int section_index = i;
    int from_stop_index = section_index;
    int to_stop_index = from_stop_index + 1;

    auto const& section = sections[section_index];
    auto const& from_stop = stops[from_stop_index];
    auto const& to_stop = stops[to_stop_index];

    auto dep_event_key = std::make_tuple(from_stop.eva_num, section.train_num,
                                         raw_to_int<uint64_t>(section.admin));
    auto arr_event_key = std::make_tuple(to_stop.eva_num, section.train_num,
                                         raw_to_int<uint64_t>(section.admin));

    resolve_platform_rule(dep_event_key, from_stop.dep.time,
                          stops_platforms[from_stop_index].dep_platforms);
    resolve_platform_rule(arr_event_key, from_stop.arr.time,
                          stops_platforms[to_stop_index].arr_platforms);
  }

  return fbb_.CreateVector(transform_to_vec(
      begin(stops_platforms), end(stops_platforms),
      [&](stop_platforms const& sp) {
        return CreatePlatformRules(fbb_, fbb_.CreateVector(sp.arr_platforms),
                                   fbb_.CreateVector(sp.dep_platforms));
      }));
}

void service_builder::resolve_platform_rule(
    platform_rule_key const& key, int time,
    std::vector<Offset<Platform>>& platforms) {
  auto dep_plr_it = shared_data_.pf_rules.find(key);
  if (dep_plr_it == end(shared_data_.pf_rules)) {
    return;
  }

  for (auto const& rule : dep_plr_it->second) {
    if (rule.time == TIME_NOT_SET || time % 1440 == rule.time) {
      platforms.push_back(
          CreatePlatform(fbb_, bb_.get_or_create_bitfield(rule.bitfield_num),
                         rule.platform_name));
    }
  }
}

Offset<Vector<int32_t>> service_builder::create_times(
    std::vector<hrd_service::stop> const& stops) {
  std::vector<int32_t> times;
  for (auto const& stop : stops) {
    times.push_back(stop.arr.time);
    times.push_back(stop.dep.time);
  }
  return fbb_.CreateVector(times);
}

}  // hrd
}  // loader
}  // motis
