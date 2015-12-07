#include <vector>
#include <iomanip>

#include "motis/realtime/statistics.h"

namespace motis {
namespace realtime {

void statistics::print(std::ostream& out) const {
  out << "statistics:\n";

  print_message_counter(_counters.messages, out);
  print_message_counter(_counters.delay_msgs, out);
  print_message_counter(_counters.delay_events, out);
  print_message_counter(_counters.delay_is, out);
  print_message_counter(_counters.delay_fc, out);
  print_message_counter(_counters.additional, out);
  print_message_counter(_counters.canceled, out);
  print_message_counter(_counters.reroutings, out);
  print_message_counter(_counters.csd, out);
  print_message_counter(_counters.unknown, out);

  out << "delay propagator: " << _ops.propagator.runs << " runs, "
      << _ops.propagator.events << " events, " << _ops.propagator.calc_max
      << " calc_max, " << _ops.propagator.updates << " updates\n"
      << "graph updater:    " << _ops.updater.time_updates << " time updates, "
      << _ops.updater.extract_route << " extract_route, "
      << _ops.updater.make_modified << " make_modified, "
      << _ops.updater.adjust_train << " adjust_train\n"
      << "delay infos:      " << _ops.delay_infos.buffered << " buffered, "
      << _ops.delay_infos.upgrades << " upgrades\n";

  out << "delay propagator: " << std::setprecision(2) << _delay_propagator.ms()
      << "ms\n"
      << "-- calc max:      " << std::setprecision(2) << _calc_max.ms()
      << "ms\n"
      << "-- queue dep:     " << std::setprecision(2) << _queue_dep.ms()
      << "ms\n"
      << "graph updater:    " << std::setprecision(2) << _graph_updater.ms()
      << "ms\n"
      << "total processing: " << std::setprecision(2) << _total_processing.ms()
      << "ms\n";
}

void statistics::print_message_counter(message_counter const& c,
                                       std::ostream& out) const {
  auto total = c.total();
  auto ignored = c.ignored();
  out << "  " << std::setw(20) << c.title() << ": " << std::setw(10) << total
      << " | " << std::setw(10) << ignored << " (" << std::setw(6)
      << std::setprecision(2) << std::fixed
      << (total == 0 ? 0.0 : (static_cast<double>(ignored) / total * 100.0))
      << "%) ignored\n";
}

void statistics::write_csv(std::ostream& out, std::time_t from,
                           std::time_t to) const {
  const char* time_format = "%Y-%m-%d %H:%M";

  char from_time[100], to_time[100];

  if (!std::strftime(from_time, sizeof(from_time), time_format,
                     std::localtime(&from))) {
    from_time[0] = '\0';
  }
  if (!std::strftime(to_time, sizeof(to_time), time_format,
                     std::localtime(&to))) {
    to_time[0] = '\0';
  }

  out << from << "," << to << "," << from_time << "," << to_time << ","
      << (to - from) / 60 << "," << std::fixed << std::setprecision(2)
      << _total_processing.ms() << "," << std::fixed << std::setprecision(2)
      << 0 /*_message_fetcher.ms()*/ << "," << std::fixed
      << std::setprecision(2) << _delay_propagator.ms() << "," << std::fixed
      << std::setprecision(2) << _graph_updater.ms() << "," << std::fixed
      << std::setprecision(2) << _calc_max.ms() << "," << std::fixed
      << std::setprecision(2) << _queue_dep.ms() << ","
      << "0,"
      << "0,"
      << "0," << _ops.propagator.runs << "," << _ops.propagator.events << ","
      << _ops.propagator.calc_max << "," << _ops.propagator.updates << ","
      << "0," << _ops.updater.time_updates << "," << _ops.updater.extract_route
      << "," << _ops.updater.make_modified << "," << _ops.updater.adjust_train
      << ","
      << "0,"
      << "0,"
      << "0," << _counters.messages.total() << ","
      << _counters.delay_msgs.total() << "," << _counters.delay_events.total()
      << "," << _counters.delay_is.total() << "," << _counters.delay_fc.total()
      << "," << _counters.additional.total() << ","
      << _counters.canceled.total() << "," << _counters.reroutings.total()
      << "," << _counters.csd.total() << "," << _counters.unknown.total() << ","
      << "0,"
      << "0,"
      << "0," << _counters.messages.ignored() << ","
      << _counters.delay_msgs.ignored() << "," << _counters.additional.ignored()
      << "," << _counters.canceled.ignored() << ","
      << _counters.reroutings.ignored() << "," << _counters.csd.ignored() << ","
      << _counters.unknown.ignored() << "\n";
}

void statistics::write_csv_header(std::ostream& out) {
  out << "t_start,"
      << "t_end,"
      << "t_start_s,"
      << "t_end_s,"
      << "t_int,"
      << "d_processing,"
      << "d_fetch,"
      << "d_propagate,"
      << "d_update,"
      << "d_calcmax,"
      << "d_queuedep,"
      << "-,"
      << "-,"
      << "-,"
      << "c_p.runs,"
      << "c_p.events,"
      << "c_p.calcmax,"
      << "c_p.updates,"
      << "0,"
      << "c_u.timeup,"
      << "c_u.extract,"
      << "c_u.makemod,"
      << "c_u.adjust,"
      << "-,"
      << "-,"
      << "-,"
      << "m_all,"
      << "m_delay,"
      << "m_d.events,"
      << "m_d.is,"
      << "m_d.fc,"
      << "m_add,"
      << "m_cancel,"
      << "m_reroute,"
      << "m_csd,"
      << "m_unknown,"
      << "-,"
      << "-,"
      << "-,"
      << "i_all,"
      << "i_delay,"
      << "i_add,"
      << "i_cancel,"
      << "i_reroute,"
      << "i_csd,"
      << "i_unknown"
      << "\n";
}

statistics operator-(statistics const& lhs, statistics const& rhs) {
  statistics s;

  s._counters.messages = lhs._counters.messages - rhs._counters.messages;
  s._counters.delay_msgs = lhs._counters.delay_msgs - rhs._counters.delay_msgs;
  s._counters.delay_events =
      lhs._counters.delay_events - rhs._counters.delay_events;
  s._counters.delay_is = lhs._counters.delay_is - rhs._counters.delay_is;
  s._counters.delay_fc = lhs._counters.delay_fc - rhs._counters.delay_fc;
  s._counters.additional = lhs._counters.additional - rhs._counters.additional;
  s._counters.canceled = lhs._counters.canceled - rhs._counters.canceled;
  s._counters.reroutings = lhs._counters.reroutings - rhs._counters.reroutings;
  s._counters.csd = lhs._counters.csd - rhs._counters.csd;
  s._counters.unknown = lhs._counters.unknown - rhs._counters.unknown;

  s._ops.propagator.runs = lhs._ops.propagator.runs - rhs._ops.propagator.runs;
  s._ops.propagator.events =
      lhs._ops.propagator.events - rhs._ops.propagator.events;
  s._ops.propagator.calc_max =
      lhs._ops.propagator.calc_max - rhs._ops.propagator.calc_max;
  s._ops.propagator.updates =
      lhs._ops.propagator.updates - rhs._ops.propagator.updates;

  s._ops.updater.time_updates =
      lhs._ops.updater.time_updates - rhs._ops.updater.time_updates;
  s._ops.updater.extract_route =
      lhs._ops.updater.extract_route - rhs._ops.updater.extract_route;
  s._ops.updater.make_modified =
      lhs._ops.updater.make_modified - rhs._ops.updater.make_modified;
  s._ops.updater.adjust_train =
      lhs._ops.updater.adjust_train - rhs._ops.updater.adjust_train;

  s._ops.delay_infos.buffered =
      lhs._ops.delay_infos.buffered - rhs._ops.delay_infos.buffered;
  s._ops.delay_infos.upgrades =
      lhs._ops.delay_infos.upgrades - rhs._ops.delay_infos.upgrades;

  s._delay_propagator = lhs._delay_propagator - rhs._delay_propagator;
  s._graph_updater = lhs._graph_updater - rhs._graph_updater;
  s._total_processing = lhs._total_processing - rhs._total_processing;

  s._calc_max = lhs._calc_max - rhs._calc_max;
  s._queue_dep = lhs._queue_dep - rhs._queue_dep;

  return s;
}

}  // namespace realtime
}  // namespace motis
