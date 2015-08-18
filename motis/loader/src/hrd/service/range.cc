#include "motis/loader/parsers/hrd/service/range.h"

#include <cassert>

#include "parser/util.h"
#include "parser/arg_parser.h"

#include "motis/loader/util.h"

using namespace parser;

namespace motis {
namespace loader {
namespace hrd {

enum event_type { ARRIVAL, DEPARTURE };

template <typename It, typename El>
It find_nth(It begin, It end, std::size_t n, El el) {
  assert(n != 0);
  std::size_t num_elements_found = 0;
  auto it = begin;
  while (it != end && num_elements_found != n) {
    it = std::find(it, end, el);
    ++num_elements_found;
  }
  return it;
}

bool is_index(cstr s) { return s[0] == '#'; }

int parse_index(cstr s) { return parse<int>(s.substr(1)); }

int get_index(hrd_service const& service, cstr eva_or_idx, cstr hhmm_or_idx,
              event_type event) {
  assert(!eva_or_idx.empty() && !hhmm_or_idx.empty());
  assert(service.eva_nums_.size() == service.events_.size());

  if (is_index(eva_or_idx)) {
    // eva_or_idx is an index which is already definite
    return parse_index(eva_or_idx);
  } else if (is_index(hhmm_or_idx) || hhmm_or_idx[0] == ' ') {
    // eva_or_idx is not an index -> eva_or_idx is an eva number
    // hhmm_or_idx is empty -> search for first occurrence
    // hhmm_or_idx is an index -> search for nth occurrence
    const auto eva_num = parse<int>(eva_or_idx);
    const auto n = is_index(hhmm_or_idx) ? parse_index(hhmm_or_idx) + 1 : 1;
    const auto it =
        find_nth(begin(service.eva_nums_), end(service.eva_nums_), n, eva_num);
    verify(it != end(service.eva_nums_),
           "%dth occurrence of eva number %d not found", n, eva_num);
    return std::distance(begin(service.eva_nums_), it);
  } else {
    const auto eva_num_searchee = parse<int>(eva_or_idx);

    // Timestamp given
    // ==> return index of the occurence
    //     of 'eva_num_searchee' where the event
    auto time_searchee = hhmm_to_min(parse<int>(hhmm_or_idx));

    unsigned idx = 0;
    auto event_it = service.events_.begin();
    for (auto const& eva_num_curr : service.eva_nums_) {
      if (eva_num_curr == eva_num_searchee) {
        int time_curr;
        switch (event) {
          case ARRIVAL:
            time_curr = event_it->first.time;
            break;
          case DEPARTURE:
            time_curr = event_it->second.time;
            break;
        }

        if (time_curr == time_searchee) {
          break;
        }
      }
      ++event_it;
      ++idx;
    }

    verify(idx < service.eva_nums_.size(), "event with time %d not found",
           time_searchee);

    return idx;
  }
}

range::range(hrd_service const& service, cstr from_eva_or_idx,
             cstr to_eva_or_idx, cstr from_hhmm_or_idx, cstr to_hhmm_or_idx)
    : from_idx(
          get_index(service, from_eva_or_idx, from_hhmm_or_idx, DEPARTURE)),
      to_idx(get_index(service, to_eva_or_idx, to_hhmm_or_idx, ARRIVAL)) {}

}  // hrd
}  // loader
}  // motis
