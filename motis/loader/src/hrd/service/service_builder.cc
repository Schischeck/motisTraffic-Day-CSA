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
    : stamm_(stamm), bitfields_(stamm.bitfields, builder), builder_(builder) {}

Offset<Category> service_builder::get_or_create_category(cstr category) {
  return get_or_create(categories_, raw_to_int<uint32_t>(category), [&]() {
    return CreateCategory(builder_, to_fbs_string(builder_, category),
                          CategoryOutputRule_CATEGORY_AND_TRAIN_NUM);
  });
}

Offset<String> service_builder::get_or_create_line_info(cstr line_info) {
  return get_or_create(line_infos_, raw_to_int<uint64_t>(line_info),
                       [&]() { return to_fbs_string(builder_, line_info); });
}

Offset<Route> service_builder::create_route(
    std::vector<hrd_service::stop> const& stops) {
  auto eva_nums =
      transform_to_vec(begin(stops), end(stops),
                       [&](hrd_service::stop const& s) { return s.eva_num; });
  return get_or_create(routes_, eva_nums, [&]() {
    return CreateRoute(builder_,
                       builder_.CreateVector(transform_to_vec(
                           begin(eva_nums), end(eva_nums), [&](int eva_num) {
                             auto it = stamm_.stations.find(eva_num);
                             verify(it != end(stamm_.stations),
                                    "station with eva number %d not found\n",
                                    eva_num);
                             return it->second;
                           })));
  });
}

Offset<Attribute> service_builder::get_or_create_attribute(
    hrd_service::attribute attr) {
  auto key = raw_to_int<uint16_t>(attr.code);
  return get_or_create(attributes_, key, [&]() {
    auto stamm_attributes_it = stamm_.attributes.find(key);
    verify(stamm_attributes_it != end(stamm_.attributes),
           "attribute with bitfield number %s not found\n", attr.code.str);
    auto text = to_fbs_string(builder_, stamm_attributes_it->second, ENCODING);
    auto fbs_attribute =
        CreateAttribute(builder_, to_fbs_string(builder_, attr.code), text,
                        bitfields_.get_or_create_bitfield(attr.bitfield_num));
    attributes_[key] = fbs_attribute;
    return fbs_attribute;
  });
}

Offset<Vector<Offset<Attribute>>> service_builder::create_attributes(
    std::vector<hrd_service::attribute> const& attributes) {
  return builder_.CreateVector(transform_to_vec(
      begin(attributes), end(attributes), [&](hrd_service::attribute const& a) {
        return get_or_create_attribute(a);
      }));
}

Offset<Vector<Offset<Section>>> service_builder::create_sections(
    std::vector<hrd_service::section> const& sections) {
  return builder_.CreateVector(transform_to_vec(
      begin(sections), end(sections), [&](hrd_service::section const& s) {
        auto attributes = create_attributes(s.attributes);
        auto category = get_or_create_category(s.category[0]);

        Offset<String> line_info;
        if (!s.line_information.empty()) {
          line_info = get_or_create_line_info(s.line_information[0]);
        }

        SectionBuilder sbuilder(builder_);
        sbuilder.add_train_nr(s.train_num);
        sbuilder.add_attributes(attributes);
        sbuilder.add_category(category);

        if (!s.line_information.empty()) {
          sbuilder.add_line_id(line_info);
        }

        return sbuilder.Finish();
      }));
}

void service_builder::resolve_platform_rule(
    platform_rule_key const& key, int time,
    std::vector<Offset<Platform>>& platforms) {
  auto dep_plr_it = stamm_.pf_rules.find(key);
  if (dep_plr_it == end(stamm_.pf_rules)) {
    return;
  }

  for (auto const& rule : dep_plr_it->second) {
    if (rule.time == TIME_NOT_SET || time % 1440 == rule.time) {
      platforms.push_back(CreatePlatform(
          builder_, bitfields_.get_or_create_bitfield(rule.bitfield_num),
          rule.platform_name));
    }
  }
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

    auto section = sections[section_index];
    auto from_stop = stops[from_stop_index];
    auto to_stop = stops[to_stop_index];

    auto dep_event_key = std::make_tuple(from_stop.eva_num, section.train_num,
                                         raw_to_int<uint64_t>(section.admin));
    auto arr_event_key = std::make_tuple(to_stop.eva_num, section.train_num,
                                         raw_to_int<uint64_t>(section.admin));

    resolve_platform_rule(dep_event_key, from_stop.dep.time,
                          stops_platforms[from_stop_index].dep_platforms);
    resolve_platform_rule(arr_event_key, from_stop.arr.time,
                          stops_platforms[to_stop_index].arr_platforms);
  }

  return builder_.CreateVector(transform_to_vec(
      begin(stops_platforms), end(stops_platforms),
      [&](stop_platforms const& sp) {
        return CreatePlatformRules(builder_,
                                   builder_.CreateVector(sp.arr_platforms),
                                   builder_.CreateVector(sp.dep_platforms));
      }));
}

Offset<Vector<int32_t>> service_builder::create_times(
    std::vector<hrd_service::stop> const& stops) {
  std::vector<int32_t> times;
  for (auto const& stop : stops) {
    times.push_back(stop.arr.time);
    times.push_back(stop.dep.time);
  }
  return builder_.CreateVector(times);
}

void service_builder::create_services(hrd_service const& s,
                                      FlatBufferBuilder& b,
                                      std::vector<Offset<Service>>& services) {
  auto expanded_services = expand_traffic_days(s, bitfields_);
  expand_repetitions(expanded_services);
  for (auto const& expanded_service : expanded_services) {
    services.push_back(CreateService(
        b, create_route(expanded_service.stops_),
        bitfields_.get_or_create_bitfield(expanded_service.traffic_days_),
        create_sections(expanded_service.sections_),
        create_platforms(expanded_service.sections_, expanded_service.stops_),
        create_times(expanded_service.stops_)));
  }
}

}  // hrd
}  // loader
}  // motis
