#pragma once

#include <ctime>
#include <iostream>
#include <chrono>

namespace motis {
namespace realtime {

class message_counter {
public:
  message_counter(std::string title) : _title(title), _total(0), _ignored(0) {}
  message_counter(std::string title, uint64_t total, uint64_t ignored)
      : _title(title), _total(total), _ignored(ignored) {}

  inline void increment() { ++_total; }
  inline void ignore() { ++_ignored; }

  inline uint64_t total() const { return _total; }
  inline uint64_t ignored() const { return _ignored; }
  explicit operator uint64_t() const { return total(); }

  inline std::string const& title() const { return _title; }

private:
  std::string _title;
  uint64_t _total;
  uint64_t _ignored;
};

inline message_counter operator-(message_counter const& lhs,
                                 message_counter const& rhs) {
  return message_counter(lhs.title(), lhs.total() - rhs.total(),
                         lhs.ignored() - rhs.ignored());
}

class operation_time_counter {
public:
  operation_time_counter() : _total(0) {}
  operation_time_counter(std::chrono::nanoseconds initial) : _total(initial) {}

  template <typename r, typename p>
  inline void add(std::chrono::duration<r, p> duration) {
    _total += duration;
  }

  inline std::chrono::nanoseconds get() const { return _total; }

  inline double ms() const {
    return std::chrono::duration_cast<std::chrono::microseconds>(_total)
               .count() /
           1000.0;
  }

private:
  std::chrono::nanoseconds _total;
};

inline operation_time_counter operator-(operation_time_counter const& lhs,
                                        operation_time_counter const& rhs) {
  return lhs.get() - rhs.get();
}

class operation_timer final {
public:
  operation_timer(operation_time_counter& counter)
      : _counter(counter), _start(std::chrono::steady_clock::now()) {}

  ~operation_timer() {
    auto stop = std::chrono::steady_clock::now();
    _counter.add(stop - _start);
  }

private:
  operation_time_counter& _counter;
  std::chrono::time_point<std::chrono::steady_clock> _start;
};

class statistics {
public:
  void print(std::ostream& out) const;
  void write_csv(std::ostream& out, std::time_t from, std::time_t to) const;

  static void write_csv_header(std::ostream& out);

  struct {
    message_counter messages{"all messages"};
    message_counter delay_msgs{"delay messages"};
    message_counter delay_events{"delay events"};
    message_counter delay_is{"delay IS"};
    message_counter delay_fc{"delay FORECAST"};
    message_counter additional{"additional"};
    message_counter canceled{"canceled"};
    message_counter reroutings{"reroutings"};
    message_counter csd{"CSD"};
    message_counter unknown{"unknown"};
  } _counters;

  using op_counter = uint64_t;
  struct {
    struct {
      op_counter runs;
      op_counter events;
      op_counter calc_max;
      op_counter updates;
    } propagator;
    struct {
      op_counter time_updates;
      op_counter extract_route;
      op_counter make_modified;
      op_counter adjust_train;
    } updater;
    // delay_infos
    // waiting_edges
    // modified_trains
  } _ops;

  operation_time_counter _delay_propagator;
  operation_time_counter _graph_updater;
  operation_time_counter _total_processing;

  // temp
  operation_time_counter _calc_max;
  operation_time_counter _queue_dep;

private:
  void print_message_counter(message_counter const& c, std::ostream& out) const;
};

statistics operator-(statistics const& lhs, statistics const& rhs);

}  // namespace realtime
}  // namespace motis
