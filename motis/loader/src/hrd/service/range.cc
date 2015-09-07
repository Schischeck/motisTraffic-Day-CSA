#include "motis/loader/parsers/hrd/service/range.h"

#include <cassert>

#include "parser/util.h"
#include "parser/arg_parser.h"

#include "motis/loader/util.h"

using namespace parser;

namespace motis {
namespace loader {
namespace hrd {

bool is_index(cstr s) { return s[0] == '#'; }

int parse_index(cstr s) { return parse<int>(s.substr(1)); }

int get_index(std::vector<hrd_service::stop> const& stops, cstr eva_or_idx,
              cstr hhmm_or_idx, bool is_departure_event) {
  assert(!eva_or_idx.empty() && !hhmm_or_idx.empty());
  if (is_index(eva_or_idx)) {
    // eva_or_idx is an index which is already definite
    return parse_index(eva_or_idx);
  } else if (is_index(hhmm_or_idx) || hhmm_or_idx.trim().len == 0) {
    // eva_or_idx is not an index -> eva_or_idx is an eva number
    // hhmm_or_idx is empty -> search for first occurrence
    // hhmm_or_idx is an index -> search for nth occurrence
    const auto eva_num = parse<int>(eva_or_idx);
    const auto n = is_index(hhmm_or_idx) ? parse_index(hhmm_or_idx) + 1 : 1;
    const auto it = find_nth(
        begin(stops), end(stops), n,
        [&](hrd_service::stop const& s) { return s.eva_num == eva_num; });
    verify(it != end(stops), "%dth occurrence of eva number %d not found", n,
           eva_num);
    return static_cast<int>(std::distance(begin(stops), it));
  } else {
    // hhmm_or_idx must be a time
    // -> return stop where eva number and time matches
    const auto eva_num = parse<int>(eva_or_idx);
    const auto time = hhmm_to_min(parse<int>(hhmm_or_idx.substr(1)));
    const auto it =
        std::find_if(begin(stops), end(stops), [&](hrd_service::stop const& s) {
          return s.eva_num == eva_num &&
                 (is_departure_event ? s.dep.time : s.arr.time) == time;
        });
    verify(it != end(stops), "event with time %d at eva number %d not found",
           time, eva_num);
    return static_cast<int>(std::distance(begin(stops), it));
  }
}

range::range(std::vector<hrd_service::stop> const& stops, cstr from_eva_or_idx,
             cstr to_eva_or_idx, cstr from_hhmm_or_idx, cstr to_hhmm_or_idx)
    : from_idx(get_index(stops, from_eva_or_idx, from_hhmm_or_idx, true)),
      to_idx(get_index(stops, to_eva_or_idx, to_hhmm_or_idx, false)) {}

}  // hrd
}  // loader
}  // motis
