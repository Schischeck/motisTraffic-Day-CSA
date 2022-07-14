#pragma once

#include <cassert>
#include <cinttypes>
#include <climits>

#include <iomanip>
#include <sstream>
#include <string>

constexpr uint16_t MAX_DAYS = 512;
constexpr uint16_t MINUTES_A_DAY = 1440;

namespace motis {

using duration = uint16_t;
using day_idx_t = int16_t;

struct time {
public:
  constexpr explicit time(int16_t day, uint16_t minute)
      : day_{static_cast<decltype(day_)>(
            day + static_cast<int16_t>(minute / MINUTES_A_DAY))},
        min_{static_cast<decltype(min_)>(minute % MINUTES_A_DAY)} {}

  time(int64_t timestamp)
      : day_{static_cast<decltype(day_)>(std::abs(timestamp) / MINUTES_A_DAY)},
        min_{static_cast<decltype(min_)>(std::abs(timestamp) % MINUTES_A_DAY)} {
    if (timestamp < 0) {
      *this = -*this;
    }
  }

  constexpr explicit time() : day_(MAX_DAYS), min_(0) {}

  constexpr inline bool valid() const {
    return day_ < MAX_DAYS && min_ < MINUTES_A_DAY;
  }

  constexpr inline int32_t ts() const { return day_ * MINUTES_A_DAY + min_; }

  time operator+(time const& o) const {
    time tmp;
    tmp.min_ = min_ + o.min_;
    tmp.day_ = day_ + o.day_ + (tmp.min_ / MINUTES_A_DAY);
    tmp.min_ %= MINUTES_A_DAY;

    assert(tmp.valid());

    return tmp;
  }

  time operator+(int32_t const o) const {
    auto tmp = time(ts() + o);

    assert(tmp.valid());

    return tmp;
  }

  time operator-(time const& o) const { return *this - o.ts(); }

  time operator-(int32_t const& o) const {
    auto tmp = time(ts() - o);

    assert(tmp.valid());

    return tmp;
  }

  time operator-() const {
    time tmp;
    if (min_ != 0) {
      tmp.day_ = -day_ - static_cast<int16_t>(1);
      tmp.min_ = MINUTES_A_DAY - min_;
      tmp.day_ -= tmp.min_ / MINUTES_A_DAY;  // if min_ == 0: subtract 1
    } else {
      tmp.day_ = -day_;
      tmp.min_ = 0;
    }

    assert(tmp.valid());

    return tmp;
  }

  bool operator<(time const& o) const { return ts() < o.ts(); }

  bool operator>(time const& o) const { return ts() > o.ts(); }

  bool operator<=(time const& o) const { return ts() <= o.ts(); }

  bool operator>=(time const& o) const { return ts() >= o.ts(); }

  bool operator<(int32_t const& o) const { return ts() < o; }

  bool operator>(int32_t const& o) const { return ts() > o; }

  bool operator<=(int32_t const& o) const { return ts() <= o; }

  bool operator>=(int32_t const& o) const { return ts() >= o; }

  bool operator==(time const& o) const {
    return day_ == o.day_ && min_ == o.min_;
  }

  bool operator!=(time const& o) const { return !operator==(o); }

  time& operator++() {
    *this = time(ts() + 1);

    assert(this->valid());

    return *this;
  }

  time& operator--() {
    *this = time(ts() - 1);

    assert(this->valid());

    return *this;
  }

  friend bool operator==(time t, int i) { return i == t.ts(); }

  friend std::ostream& operator<<(std::ostream& out, time const& t) {
    return !t.valid() ? (out << "INVALID")
                      : (out << std::setfill('0') << std::setw(3) << t.day_
                             << "." << std::setfill('0') << std::setw(2)
                             << (t.min_ / 60) << ":" << std::setfill('0')
                             << std::setw(2) << (t.min_ % 60));
  }

  std::string to_str() const {
    std::stringstream out;
    out << *this;
    return out.str();
  }

  int16_t day() const {
    assert(day_ <= MAX_DAYS);
    return day_;
  }
  uint16_t mam() const {
    assert(min_ < MINUTES_A_DAY);
    return min_;
  }

private:
  int16_t day_;
  uint16_t min_;
};

constexpr time INVALID_TIME = time();
constexpr uint32_t SCHEDULE_OFFSET_DAYS = 5;
constexpr uint32_t SCHEDULE_OFFSET_MINUTES =
    SCHEDULE_OFFSET_DAYS * MINUTES_A_DAY;

inline time to_motis_time(int minutes) {
  // plus four days, because the maximum journey duration is 4 days
  // plus one day, because the first valid motis timestamp is MINUTES_A_DAY
  return time(SCHEDULE_OFFSET_MINUTES + minutes);
}

inline time to_motis_time(int day_index, int minutes) {
  return to_motis_time(day_index * MINUTES_A_DAY + minutes);
}

inline time to_motis_time(int day_index, int hours, int minutes) {
  return to_motis_time(day_index, hours * 60 + minutes);
}

inline std::string format_time(time t) {
  if (t == INVALID_TIME) {
    return "INVALID";
  }

  int day = t.day();
  int minutes = t.mam();

  std::ostringstream out;
  out << std::setw(2) << std::setfill('0') << (minutes / 60) << ":"
      << std::setw(2) << std::setfill('0') << (minutes % 60) << "." << day;

  return out.str();
}

inline std::time_t motis_to_unixtime(std::time_t schedule_begin, time t) {
  return schedule_begin + t.ts() * 60;
}

inline time unix_to_motistime(std::time_t schedule_begin, std::time_t t) {
  if (t < schedule_begin) {
    return INVALID_TIME;
  }
  auto motistime = time((t - schedule_begin) / 60);
  if (!motistime.valid()) {
    return INVALID_TIME;
  }
  return motistime;
}

}  // namespace motis
