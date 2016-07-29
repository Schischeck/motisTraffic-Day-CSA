#pragma once

#include <numeric>

#include "motis/core/journey/journey.h"
#include "motis/routing/label/configs.h"
#include "motis/routing/output/label_chain_parser.h"
#include "motis/routing/output/stop.h"
#include "motis/routing/output/to_journey.h"
#include "motis/routing/output/transport.h"

namespace motis {

struct schedule;

namespace routing {
namespace output {

template <typename Label>
inline unsigned db_costs(Label const&) {
  return 0;
}

template <search_dir Dir>
inline unsigned db_costs(late_connections_label<Dir> const& l) {
  return l.db_costs_;
}

template <search_dir Dir>
inline unsigned db_costs(late_connections_label_for_tests<Dir> const& l) {
  return l.db_costs_;
}

template <typename Label>
inline unsigned night_penalty(Label const&) {
  return 0;
}

template <search_dir Dir>
inline unsigned night_penalty(late_connections_label<Dir> const& l) {
  return l.night_penalty_;
}

template <search_dir Dir>
inline unsigned night_penalty(late_connections_label_for_tests<Dir> const& l) {
  return l.night_penalty_;
}

template <typename Label>
journey labels_to_journey(schedule const& sched, Label* label,
                          search_dir const dir) {
  auto parsed = parse_label_chain(sched, label, dir);
  std::vector<intermediate::stop> const& s = parsed.first;
  std::vector<intermediate::transport> const& t = parsed.second;

  journey j;
  j.stops_ = generate_journey_stops(s, sched);
  j.transports_ = generate_journey_transports(t, sched);
  j.trips_ = generate_journey_trips(t, sched);
  j.attributes_ = generate_journey_attributes(t);

  j.duration_ = label->now_ - label->start_;
  j.transfers_ = std::accumulate(
      begin(j.stops_), end(j.stops_), 0,
      [](int transfers_count, journey::stop const& s) {
        return s.interchange_ ? transfers_count + 1 : transfers_count;
      });

  j.db_costs_ = db_costs(*label);
  j.night_penalty_ = night_penalty(*label);

  return j;
}

}  // namespace output
}  // namespace routing
}  // namespace motis
