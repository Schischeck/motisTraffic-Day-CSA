#include "motis/loader/builders/hrd/service_builder.h"

#include "motis/loader/util.h"
#include "motis/loader/parsers/hrd/files.h"

using namespace parser;
using namespace flatbuffers;

namespace motis {
namespace loader {
namespace hrd {

service_builder::service_builder(platform_rules plf_rules)
    : plf_rules_(std::move(plf_rules)) {}

Offset<Vector<Offset<Section>>> create_sections(
    std::vector<hrd_service::section> const& sections, category_builder& cb,
    provider_builder& pb, line_builder& lb, attribute_builder& ab,
    bitfield_builder& bb, direction_builder& db, station_builder& sb,
    FlatBufferBuilder& fbb) {
  return fbb.CreateVector(transform_to_vec(
      begin(sections), end(sections), [&](hrd_service::section const& s) {
        return CreateSection(
            fbb, cb.get_or_create_category(s.category[0], fbb),
            pb.get_or_create_provider(raw_to_int<uint64_t>(s.admin), fbb),
            s.train_num, lb.get_or_create_line(s.line_information, fbb),
            ab.create_attributes(s.attributes, bb, fbb),
            db.get_or_create_direction(s.directions, sb, fbb));
      }));
}

void create_platforms(platform_rule_key const& key, int time,
                      platform_rules const& plf_rules, bitfield_builder& bb,
                      std::vector<Offset<Platform>>& platforms,
                      FlatBufferBuilder& fbb) {
  auto dep_plr_it = plf_rules.find(key);
  if (dep_plr_it == end(plf_rules)) {
    return;
  }

  for (auto const& rule : dep_plr_it->second) {
    if (rule.time == TIME_NOT_SET || time % 1440 == rule.time) {
      platforms.push_back(
          CreatePlatform(fbb, bb.get_or_create_bitfield(rule.bitfield_num, fbb),
                         rule.platform_name));
    }
  }
}

Offset<Vector<Offset<PlatformRules>>> create_platforms(
    std::vector<hrd_service::section> const& sections,
    std::vector<hrd_service::stop> const& stops,
    platform_rules const& plf_rules, bitfield_builder& bb,
    FlatBufferBuilder& fbb) {
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

    create_platforms(dep_event_key, from_stop.dep.time, plf_rules, bb,
                     stops_platforms[from_stop_index].dep_platforms, fbb);
    create_platforms(arr_event_key, from_stop.arr.time, plf_rules, bb,
                     stops_platforms[to_stop_index].arr_platforms, fbb);
  }

  return fbb.CreateVector(transform_to_vec(
      begin(stops_platforms), end(stops_platforms),
      [&](stop_platforms const& sp) {
        return CreatePlatformRules(fbb, fbb.CreateVector(sp.arr_platforms),
                                   fbb.CreateVector(sp.dep_platforms));
      }));
}

Offset<Vector<int32_t>> create_times(
    std::vector<hrd_service::stop> const& stops, FlatBufferBuilder& fbb) {
  std::vector<int32_t> times;
  for (auto const& stop : stops) {
    times.push_back(stop.arr.time);
    times.push_back(stop.dep.time);
  }
  return fbb.CreateVector(times);
}

void service_builder::create_service(hrd_service const& s, route_builder& rb,
                                     station_builder& sb, category_builder& cb,
                                     provider_builder& pb, line_builder& lb,
                                     attribute_builder& ab,
                                     bitfield_builder& bb,
                                     direction_builder& db,
                                     FlatBufferBuilder& fbb) {
  fbs_services_.push_back(CreateService(
      fbb, rb.get_or_create_route(s.stops_, sb, fbb),
      bb.get_or_create_bitfield(s.traffic_days_, fbb),
      create_sections(s.sections_, cb, pb, lb, ab, bb, db, sb, fbb),
      create_platforms(s.sections_, s.stops_, plf_rules_, bb, fbb),
      create_times(s.stops_, fbb)));
}

}  // hrd
}  // loader
}  // motis
