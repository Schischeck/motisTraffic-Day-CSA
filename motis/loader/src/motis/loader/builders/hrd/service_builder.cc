#include "motis/loader/parsers/hrd/service/service_builder.h"

#include "motis/loader/util.h"
#include "motis/loader/parsers/hrd/files.h"
#include "motis/loader/parsers/hrd/service/split_service.h"
#include "motis/loader/parsers/hrd/service/repeat_service.h"

using namespace parser;
using namespace flatbuffers;

namespace motis {
namespace loader {
namespace hrd {

service_builder::service_builder(shared_data const& stamm,
                                 FlatBufferBuilder& builder)
    : shared_data_(stamm),
      bb_(stamm.bitfields, builder),
      sb_(stamm.stations, builder),
      pb_(stamm.providers, builder),
      fbb_(builder) {}

void service_builder::build_service(hrd_service const& s) {
  auto const route = create_route(s.stops_);
  services_.push_back(CreateService(
      fbb_, route, bb_.get_or_create_bitfield(s.traffic_days_),
      create_sections(s.sections_), create_platforms(s.sections_, s.stops_),
      create_times(s.stops_)));
}

Offset<Route> service_builder::create_route(
    std::vector<hrd_service::stop> const& stops) {
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
                    return sb_.get_or_create_station(std::get<0>(ev));
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

// Offset<Category> service_builder::get_or_create_category(cstr category) {
//  auto const category_key = raw_to_int<uint32_t>(category);
//
//  auto it = shared_data_.categories.find(category_key);
//  verify(it != end(shared_data_.categories), "missing category: %.*s",
//         static_cast<int>(category.length()), category.c_str());
//
//  return get_or_create(categories_, category_key, [&]() {
//    return CreateCategory(fbb_, to_fbs_string(fbb_, it->second.name),
//                          it->second.output_rule);
//  });
//}

// Offset<Vector<Offset<Attribute>>> service_builder::create_attributes(
//    std::vector<hrd_service::attribute> const& attributes) {
//  return fbb_.CreateVector(transform_to_vec(
//      begin(attributes), end(attributes), [&](hrd_service::attribute const& a)
//      {
//        return get_or_create_attribute(a);
//      }));
//}

// Offset<Attribute> service_builder::get_or_create_attribute(
//    hrd_service::attribute attr) {
//  auto const attr_info_key = raw_to_int<uint16_t>(attr.code);
//  auto const attr_key = std::make_pair(attr_info_key, attr.bitfield_num);
//
//  return get_or_create(attributes_, attr_key, [&]() {
//    auto const attr_info =
//        get_or_create(attribute_infos_, attr_info_key, [&]() {
//          auto const stamm_attributes_it =
//              shared_data_.attributes.find(attr_info_key);
//          verify(stamm_attributes_it != end(shared_data_.attributes),
//                 "attribute with code %.*s not found\n",
//                 static_cast<int>(attr.code.length()), attr.code.c_str());
//
//          auto const fbs_attribute_info = CreateAttributeInfo(
//              fbb_, to_fbs_string(fbb_, attr.code),
//              to_fbs_string(fbb_, stamm_attributes_it->second, ENCODING));
//
//          attribute_infos_[attr_info_key] = fbs_attribute_info;
//          return fbs_attribute_info;
//        });
//
//    auto const fbs_attribute = CreateAttribute(
//        fbb_, attr_info, bb_.get_or_create_bitfield(attr.bitfield_num));
//    attributes_[attr_key] = fbs_attribute;
//    return fbs_attribute;
//  });
//}

// Offset<String> service_builder::get_or_create_line_info(
//    std::vector<cstr> const& line_info) {
//  if (line_info.empty()) {
//    return 0;
//  } else {
//    return get_or_create(line_infos_, raw_to_int<uint64_t>(line_info[0]),
//                         [&]() { return to_fbs_string(fbb_, line_info[0]); });
//  }
//}

// Offset<Direction> service_builder::get_or_create_direction(
//    std::vector<std::pair<uint64_t, int>> const& directions) {
//  if (directions.empty()) {
//    return 0;
//  } else {
//    auto const direction_key = directions[0];
//    return get_or_create(directions_, direction_key.first, [&]() {
//      switch (direction_key.second) {
//        case hrd_service::EVA_NUMBER:
//          return CreateDirection(
//              fbb_, sb_.get_or_create_station(direction_key.first));
//        case hrd_service::DIRECTION_CODE: {
//          auto it = shared_data_.directions.find(direction_key.first);
//          verify(it != end(shared_data_.directions),
//                 "missing direction info: %lu", direction_key.first);
//          return CreateDirection(fbb_, 0,
//                                 to_fbs_string(fbb_, it->second, ENCODING));
//        }
//        default: assert(false); return Offset<Direction>(0);
//      }
//    });
//  }
//}

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
