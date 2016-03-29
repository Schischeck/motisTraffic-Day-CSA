#pragma once

#include <ostream>

namespace motis {
namespace rt {
namespace handler {

struct trip_counter {
 friend std::ostream& operator<<(std::ostream&, trip_counter const&);

  uint64_t total = 0;
  uint64_t found = 0;

  uint64_t missed_primary = 0;
  uint64_t fuzzy_primary = 0;

  // primary excuses ;)
  uint64_t is_additional = 0;
  uint64_t primary_not_in_schedule = 0;

  uint64_t missed_secondary = 0;
  uint64_t secondary_not_in_schedule = 0;
};

struct message_counter {
public:
  message_counter(std::string title) : title_(std::move(title)), count_(0) {}
  // message_message_counter(std::string title, uint64_t total, uint64_t ignored)
  //     : _title(title), _total(total), _ignored(ignored) {}

  inline void inc() { ++count_; }
  // inline void ignore() { ++_ignored; }

  // inline uint64_t total() const { return _total; }
  // inline uint64_t ignored() const { return _ignored; }
  // explicit operator uint64_t() const { return total(); }

  // inline std::string const& title() const { return _title; }

  friend std::ostream& operator<<(std::ostream&, message_counter const&);

  std::string title_;
  uint64_t count_;
  uint64_t ds100_;
  trip_counter trips_;
};

struct statistics {
  friend std::ostream& operator<<(std::ostream&, statistics const&);


  message_counter delay{"delay"};
  message_counter additional{"additional"};
  message_counter canceled{"canceled"};
  message_counter reroute{"reroutings"};
  message_counter assessment{"assessment"};
  message_counter decision{"decision"};
};

}  // namespace handler
}  // namespace rt
}  // namespace motis
