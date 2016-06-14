#include <algorithm>
#include <limits>
#include <vector>

#include "motis/core/schedule/delay_info.h"

namespace motis {
namespace rt {

struct entry;
using entries = std::vector<entry>;

constexpr auto tmax = std::numeric_limits<motis::time>::max();
constexpr auto tmin = std::numeric_limits<motis::time>::min();

struct entry : public delay_info {
  entry(delay_info const& di) : delay_info(di), min_(tmin), max_(tmax) {}

  void correct(entry& e) {
    if (e.get_current_time() > e.max_) {
      e.set(timestamp_reason::REPAIR, e.max_);
    }
    if (e.get_current_time() < e.min_) {
      e.set(timestamp_reason::REPAIR, e.min_);
    }
  }

  motis::time min_, max_;
};

void apply_is(entries& v, int index) {
  if (v[index].get_reason() != timestamp_reason::IS) {
    return;
  }

  auto const is = v[index].get_is_time();
  for (int i = index; i < v.size(); ++i) {
    v[i].min_ = std::max(v[i].min_, is);
  }
  for (int i = index; i >= 0; --i) {
    v[i].max_ = std::min(v[i].max_, is);
  }
}

void set_min(std::set<edge const*> const& edges) {
  for (auto const& e : edges) {
  }
}

void fix_times(ev_key const& k) { std::map<> }

}  // namespace rt
}  // namespace motis
