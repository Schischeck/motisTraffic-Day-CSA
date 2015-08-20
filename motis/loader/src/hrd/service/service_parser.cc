#include "motis/loader/parsers/hrd/service/service_parser.h"

#include <cctype>
#include <algorithm>

#include "parser/util.h"

#include "motis/loader/util.h"
#include "motis/loader/parser_error.h"

#include "motis/loader/parsers/hrd/service/specification.h"
#include "motis/loader/parsers/hrd/service/hrd_service.h"
#include "motis/loader/util.h"

using namespace parser;
using namespace flatbuffers;

namespace motis {
namespace loader {
namespace hrd {

struct service_builder {

  service_builder(shared_data const& stamm) : stamm_(stamm) {}

  Offset<String> get_or_create_category(cstr category) {
    auto key = raw_to_int<uint32_t>(category);
    auto it = categories_.find(key);
    if (it != end(categories_)) {
      return it->second;
    } else {
      auto fbs_category = to_fbs_string(builder_, category);
      categories_[key] = fbs_category;
      return fbs_category;
    }
  }

  Offset<String> get_or_create_line_info(cstr line_info) {
    auto key = raw_to_int<uint64_t>(line_info);
    auto it = line_infos_.find(key);
    if (it != end(line_infos_)) {
      return it->second;
    } else {
      auto fbs_line_info = to_fbs_string(builder_, line_info);
      line_infos_[key] = fbs_line_info;
      return fbs_line_info;
    }
  }

  Offset<Route> create_route(std::vector<hrd_service::stop> const& stops) {
    auto eva_nums = transform<std::vector<int>>(
        begin(stops), end(stops),
        [&](hrd_service::stop const& s) { return s.eva_num; });
    auto it = routes_.find(eva_nums);
    if (it == end(routes_)) {
      return routes_
          .emplace(eva_nums,
                   CreateRoute(
                       builder_,
                       builder_.CreateVector(transform_to_vec(
                           begin(eva_nums), end(eva_nums), [&](int eva_num) {
                             auto it = stamm_.stations.find(eva_num);
                             verify(it != end(stamm_.stations),
                                    "station with eva number %d not found\n",
                                    eva_num);
                             return it->second;
                           }))))
          .first->second;
    } else {
      return it->second;
    }
  }

  Offset<Attribute> get_or_create_attribute(hrd_service::attribute attr) {
    auto key = raw_to_int<uint16_t>(attr.code);
    auto fbs_attributes_it = attributes_.find(key);
    if (fbs_attributes_it != end(attributes_)) {
      return fbs_attributes_it->second;
    } else {
      auto stamm_attributes_it = stamm_.attributes.find(key);
      verify(stamm_attributes_it != end(stamm_.attributes),
             "attribute with bitfield number %s not found\n", attr.code.str);
      auto text = builder_.CreateString(stamm_attributes_it->second);
      auto fbs_attribute =
          CreateAttribute(builder_, to_fbs_string(builder_, attr.code), text,
                          get_traffic_days(attr.bitfield_num));
      attributes_[key] = fbs_attribute;
      return fbs_attribute;
    }
  }

  Offset<Vector<Offset<Attribute>>> create_attributes(
      std::vector<hrd_service::attribute> const& attributes) {
    return builder_.CreateVector(
        transform_to_vec(begin(attributes), end(attributes),
                         [&](hrd_service::attribute const& a) {
          return get_or_create_attribute(a);
        }));
  }

  Offset<String> get_traffic_days(int bitfield_num) {
    auto it = stamm_.bitfields.find(bitfield_num);
    verify(it != end(stamm_.bitfields),
           "bitfield with bitfield number %d not found\n", bitfield_num);
    return it->second;
  }

  Offset<Vector<Offset<Section>>> create_sections(
      std::vector<hrd_service::section> const& sections) {
    return builder_.CreateVector(transform_to_vec(
        begin(sections), end(sections), [&](hrd_service::section const& s) {
          return CreateSection(builder_, s.train_num,
                               get_or_create_category(s.category[0]),
                               get_or_create_line_info(s.line_information[0]),
                               create_attributes(s.attributes),
                               get_traffic_days(s.traffic_days[0]));
        }));
  }

  Offset<Vector<Offset<Platform>>> create_platforms(
      std::vector<hrd_service::section> const& sections,
      std::vector<hrd_service::stop> const& stops) {
    std::vector<Offset<Platform>> platforms;
    for (int i = 1; i < stops.size(); ++i) {
      auto section = sections[i - 1];
      auto prev_stop = stops[i - 1];
      auto curr_stop = stops[i];
      auto dep_event_key = std::make_tuple(prev_stop.eva_num, section.train_num,
                                           raw_to_int<uint64_t>(section.admin));
      auto arr_event_key = std::make_tuple(curr_stop.eva_num, section.train_num,
                                           raw_to_int<uint64_t>(section.admin));
    }
    return builder_.CreateVector(platforms);
  }

  void create_services(hrd_service const& s, FlatBufferBuilder& b,
                       std::vector<Offset<Service>>& services) {
    services.push_back(
        CreateService(b, create_route(s.stops_), create_sections(s.sections_),
                      create_platforms(s.sections_, s.stops_), {}));
  }

  shared_data const& stamm_;
  std::map<uint16_t, Offset<Attribute>> attributes_;
  std::map<uint32_t, Offset<String>> categories_;
  std::map<uint64_t, Offset<String>> line_infos_;
  std::map<std::vector<int>, Offset<Route>> routes_;
  FlatBufferBuilder builder_;
};

void parse_services(loaded_file file, shared_data const& stamm,
                    FlatBufferBuilder& fbb,
                    std::vector<Offset<Service>>& services) {
  service_builder sb(stamm);
  specification spec;
  for_each_line_numbered(file.content, [&](cstr line, int line_number) {
    bool finished = spec.read_line(line, file.name, line_number);

    if (!finished) {
      return;
    }

    if (!spec.valid()) {
      throw parser_error(file.name, line_number);
    }

    // Store if relevant.
    if (!spec.ignore()) {
      try {
        sb.create_services(hrd_service(spec), fbb, services);
      } catch (std::runtime_error const& e) {
        throw parser_error(file.name, line_number);
      }
    }

    // Next try! Re-read first line of next service.
    spec.reset();
    spec.read_line(line, file.name, line_number);
  });
}

}  // hrd
}  // loader
}  // motis
