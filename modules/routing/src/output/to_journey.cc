#include "motis/routing/output/to_journey.h"

#include <string>

#include "boost/lexical_cast.hpp"

#include "motis/routing/output/interval_map.h"

namespace motis {
namespace routing {
namespace output {

int output_train_nr(uint32_t const& train_nr, uint32_t original_train_nr) {
  return train_nr <= 999999 ? train_nr : original_train_nr;
}

journey::transport generate_journey_transport(
    unsigned int from, unsigned int to, connection_info const* con_info,
    schedule const& sched, unsigned int route_id = 0, duration duration = 0,
    int slot = 0) {
  journey::transport::transport_type type;
  std::string name;
  std::string cat_name;
  unsigned cat_id = 0;
  unsigned clasz = 0;
  unsigned train_nr = 0;
  std::string line_identifier;
  std::string direction;
  std::string provider;

  if (con_info == nullptr) {
    type = journey::transport::Walk;
  } else {
    type = journey::transport::PublicTransport;
    std::string print_train_nr;

    cat_id = con_info->family;
    cat_name = sched.categories[con_info->family]->name;

    auto clasz_it = sched.classes.find(cat_name);
    clasz = clasz_it == end(sched.classes) ? 9 : clasz_it->second;

    line_identifier = con_info->line_identifier;

    train_nr = output_train_nr(con_info->train_nr, con_info->original_train_nr);
    if (train_nr != 0) {
      print_train_nr = boost::lexical_cast<std::string>(train_nr);
    } else if (train_nr == 0 && !line_identifier.empty()) {
      print_train_nr = line_identifier;
    } else {
      print_train_nr = "";
    }

    if (con_info->dir_ != nullptr) {
      direction = *con_info->dir_;
    }

    if (con_info->provider_ != nullptr) {
      provider = con_info->provider_->full_name;
    }

    switch (sched.categories[con_info->family]->output_rule) {
      case category::CATEGORY_AND_TRAIN_NUM:
        name = cat_name + " " + print_train_nr;
        break;

      case category::CATEGORY: name = cat_name; break;

      case category::TRAIN_NUM: name = print_train_nr; break;

      case category::NOTHING: break;

      case category::PROVIDER_AND_TRAIN_NUM:
        if (con_info->provider_ != nullptr) {
          name = con_info->provider_->short_name + " ";
        }
        name += print_train_nr;
        break;

      case category::PROVIDER:
        if (con_info->provider_ != nullptr) {
          name = con_info->provider_->short_name;
        }
        break;

      case category::CATEGORY_AND_LINE:
        name = cat_name + " " + line_identifier;
        break;
    }
  }

  return {from,     to,       type /* TODO: mumo */,
          name,     cat_name, cat_id,
          clasz,    train_nr, line_identifier,
          duration, slot,     direction,
          provider, route_id, "" /* TODO mumo-type-name */,
          0 /* TODO mumo-price */};
}

std::vector<journey::transport> generate_journey_transports(
    std::vector<intermediate::transport> const& transports,
    schedule const& sched) {
  struct con_info_cmp {
    bool operator()(connection_info const* a, connection_info const* b) const {
      auto train_nr_a = output_train_nr(a->train_nr, a->original_train_nr);
      auto train_nr_b = output_train_nr(b->train_nr, b->original_train_nr);
      return std::tie(a->line_identifier, a->family, train_nr_a, a->dir_) <
             std::tie(b->line_identifier, b->family, train_nr_b, b->dir_);
    }
  };

  std::map<connection_info const*, int> route_ids;
  std::vector<journey::transport> journey_transports;
  interval_map<connection_info const*, con_info_cmp> intervals;
  for (auto const& t : transports) {
    if (t.con) {
      auto con_info = t.con->_full_con->con_info;
      while (con_info) {
        intervals.add_entry(con_info, t.from, t.to);
        route_ids.emplace(con_info, t.route_id);
        con_info = con_info->merged_with;
      }
    } else {
      journey_transports.push_back(generate_journey_transport(
          t.from, t.to, nullptr, sched, t.duration, t.slot));
    }
  }

  for (auto const& t : intervals.get_attribute_ranges()) {
    for (auto const& range : t.second) {
      journey_transports.push_back(generate_journey_transport(
          range.from, range.to, t.first, sched, route_ids.at(t.first)));
    }
  }

  std::sort(begin(journey_transports), end(journey_transports),
            [](journey::transport const& lhs, journey::transport const& rhs) {
              return lhs.from < rhs.from;
            });

  return journey_transports;
}

std::vector<journey::stop> generate_journey_stops(
    std::vector<intermediate::stop> const& stops, schedule const& sched) {
  std::vector<journey::stop> journey_stops;
  for (auto const& stop : stops) {
    journey_stops.push_back(
        {stop.index, stop.interchange, sched.stations[stop.station_id]->name,
         sched.stations[stop.station_id]->eva_nr,
         sched.stations[stop.station_id]->width,
         sched.stations[stop.station_id]->length,
         stop.a_time != INVALID_TIME
             ? journey::stop::event_info{true, motis_to_unixtime(
                                                   sched.schedule_begin_,
                                                   stop.a_time),
                                         motis_to_unixtime(
                                             sched.schedule_begin_,
                                             stop.a_time),
                                         sched.tracks[stop.a_platform]}
             : journey::stop::event_info{false, 0, 0, ""},
         stop.d_time != INVALID_TIME
             ? journey::stop::event_info{true, motis_to_unixtime(
                                                   sched.schedule_begin_,
                                                   stop.d_time),
                                         motis_to_unixtime(
                                             sched.schedule_begin_,
                                             stop.d_time),
                                         sched.tracks[stop.d_platform]}
             : journey::stop::event_info{false, 0, 0, ""}});
  }
  return journey_stops;
}

std::vector<journey::attribute> generate_journey_attributes(
    std::vector<intermediate::transport> const& transports) {
  interval_map<attribute const*> attributes;
  for (auto const& transport : transports) {
    if (transport.con == nullptr) {
      continue;
    } else {
      for (auto const& attribute :
           transport.con->_full_con->con_info->attributes) {
        attributes.add_entry(attribute, transport.from, transport.to);
      }
    }
  }

  std::vector<journey::attribute> journey_attributes;
  for (auto const& attribute_range : attributes.get_attribute_ranges()) {
    auto const& attribute = attribute_range.first;
    auto const& attribute_ranges = attribute_range.second;
    auto const& code = attribute->_code;
    auto const& text = attribute->_str;

    for (auto const& range : attribute_ranges) {
      journey_attributes.push_back({static_cast<unsigned>(range.from),
                                    static_cast<unsigned>(range.to), code,
                                    text});
    }
  }

  return journey_attributes;
}

}  // namespace output
}  // namespace routing
}  // namespace motis
