#include "motis/loader/hrd/builder/service_builder.h"

#include <sstream>

#include "motis/core/common/get_or_create.h"
#include "motis/loader/hrd/files.h"
#include "motis/loader/util.h"

#if defined(_WIN32) && defined(CreateService)
#undef CreateService
#endif

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
            fbb, cb.get_or_create_category(s.category_[0], fbb),
            pb.get_or_create_provider(raw_to_int<uint64_t>(s.admin_), fbb),
            s.train_num_, lb.get_or_create_line(s.line_information_, fbb),
            ab.create_attributes(s.attributes_, bb, fbb),
            db.get_or_create_direction(s.directions_, sb, fbb));
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
    if (rule.time_ == TIME_NOT_SET || time % 1440 == rule.time_) {
      platforms.push_back(CreatePlatform(
          fbb, bb.get_or_create_bitfield(rule.bitfield_num_, fbb),
          rule.platform_name_));
    }
  }
}

Offset<Vector<Offset<PlatformRules>>> create_platforms(
    std::vector<hrd_service::section> const& sections,
    std::vector<hrd_service::stop> const& stops,
    platform_rules const& plf_rules, bitfield_builder& bb,
    FlatBufferBuilder& fbb) {
  struct stop_platforms {
    std::vector<Offset<Platform>> dep_platforms_;
    std::vector<Offset<Platform>> arr_platforms_;
  };

  std::vector<stop_platforms> stops_platforms(stops.size());
  for (unsigned i = 0; i < sections.size(); ++i) {
    int section_index = i;
    int from_stop_index = section_index;
    int to_stop_index = from_stop_index + 1;

    auto const& section = sections[section_index];
    auto const& from_stop = stops[from_stop_index];
    auto const& to_stop = stops[to_stop_index];

    auto dep_event_key = std::make_tuple(from_stop.eva_num_, section.train_num_,
                                         raw_to_int<uint64_t>(section.admin_));
    auto arr_event_key = std::make_tuple(to_stop.eva_num_, section.train_num_,
                                         raw_to_int<uint64_t>(section.admin_));

    create_platforms(dep_event_key, from_stop.dep_.time_, plf_rules, bb,
                     stops_platforms[from_stop_index].dep_platforms_, fbb);
    create_platforms(arr_event_key, to_stop.arr_.time_, plf_rules, bb,
                     stops_platforms[to_stop_index].arr_platforms_, fbb);
  }

  return fbb.CreateVector(transform_to_vec(
      begin(stops_platforms), end(stops_platforms),
      [&](stop_platforms const& sp) {
        return CreatePlatformRules(fbb, fbb.CreateVector(sp.arr_platforms_),
                                   fbb.CreateVector(sp.dep_platforms_));
      }));
}

Offset<Vector<int32_t>> create_times(
    std::vector<hrd_service::stop> const& stops, FlatBufferBuilder& fbb) {
  std::vector<int32_t> times;
  for (auto const& stop : stops) {
    times.push_back(stop.arr_.time_);
    times.push_back(stop.dep_.time_);
  }
  return fbb.CreateVector(times);
}

Offset<Service> service_builder::create_service(
    hrd_service const& s, route_builder& rb, station_builder& sb,
    category_builder& cb, provider_builder& pb, line_builder& lb,
    attribute_builder& ab, bitfield_builder& bb, direction_builder& db,
    FlatBufferBuilder& fbb, bool is_rule_participant) {
  fbs_services_.push_back(CreateService(
      fbb, rb.get_or_create_route(s.stops_, sb, fbb),
      bb.get_or_create_bitfield(s.traffic_days_, fbb),
      create_sections(s.sections_, cb, pb, lb, ab, bb, db, sb, fbb),
      create_platforms(s.sections_, s.stops_, plf_rules_, bb, fbb),
      create_times(s.stops_, fbb), rb.get_or_create_route(s.stops_, sb, fbb).o,
      CreateServiceDebugInfo(fbb, get_or_create(filenames_, s.origin_.filename_,
                                                [&fbb, &s]() {
                                                  return fbb.CreateString(
                                                      s.origin_.filename_);
                                                }),
                             s.origin_.line_number_from_),
      static_cast<uint8_t>(is_rule_participant ? 1u : 0u),
      s.initial_train_num_));
  return fbs_services_.back();
}

}  // namespace hrd
}  // namespace loader
}  // namespace motis
