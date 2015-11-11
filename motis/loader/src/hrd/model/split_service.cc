#include "motis/loader/hrd/model/split_service.h"

#include <cassert>
#include <algorithm>
#include <iterator>

#include "parser/util.h"

#include "motis/loader/util.h"
#include "motis/loader/hrd/builder/bitfield_builder.h"

namespace motis {
namespace loader {
namespace hrd {

bitfield all = create_uniform_bitfield<BIT_COUNT>('1');
bitfield none = create_uniform_bitfield<BIT_COUNT>('0');

struct split_info {
  bitfield traffic_days;
  int from_section_idx, to_section_idx;
};

struct splitter {
  void check_and_remember(int start, int pos, bitfield const& b) {
    for (auto const& w : written) {
      verify((b & w.traffic_days) == none, "invalid bitfields");
    }
    written.push_back({b, start, pos - 1});
  }

  void write_and_remove(std::vector<bitfield>& sections, unsigned start,
                        unsigned pos, bitfield current) {
    if (current != none) {
      auto not_current = (~current);
      for (unsigned i = start; i < pos; ++i) {
        sections[i] &= not_current;
      }
      check_and_remember(start, pos, current);
    }
  }

  void split(std::vector<bitfield>& sections, unsigned start, unsigned pos,
             bitfield current) {
    if (pos == sections.size()) {
      write_and_remove(sections, start, pos, current);
      return;
    }

    auto intersection = current & sections[pos];
    if (intersection == none) {
      write_and_remove(sections, start, pos, current);
      return;
    }

    split(sections, start, pos + 1, intersection);
    auto const diff = current & (~intersection);
    write_and_remove(sections, start, pos, diff);
  }

  std::vector<split_info> split(std::vector<bitfield>& sections) {
    for (unsigned pos = 0; pos < sections.size(); ++pos) {
      split(sections, pos, pos, sections[pos]);
    }
    return written;
  }

  std::vector<split_info> written;
};

std::vector<split_info> split(hrd_service const& s,
                              std::map<int, bitfield> const& bitfields) {
  auto section_bitfields = transform_to_vec(
      begin(s.sections_), end(s.sections_), [&](hrd_service::section const& s) {
        auto it = bitfields.find(s.traffic_days[0]);
        verify(it != end(bitfields), "bitfield not found");
        return it->second;
      });
  return splitter().split(section_bitfields);
}

hrd_service new_service_from_split(split_info const& s,
                                   hrd_service const& origin) {
  auto number_of_stops = s.to_section_idx - s.from_section_idx + 2;
  std::vector<hrd_service::stop> stops(number_of_stops);
  std::copy(std::next(begin(origin.stops_), s.from_section_idx),
            std::next(begin(origin.stops_), s.to_section_idx + 2),
            begin(stops));

  auto number_of_sections = s.to_section_idx - s.from_section_idx + 1;
  std::vector<hrd_service::section> sections(number_of_sections);
  std::copy(std::next(begin(origin.sections_), s.from_section_idx),
            std::next(begin(origin.sections_), s.to_section_idx + 1),
            begin(sections));

  return hrd_service(origin.origin_, origin.num_repetitions_, origin.interval_,
                     stops, sections, s.traffic_days);
}

void expand_traffic_days(hrd_service const& service,
                         std::map<int, bitfield> const& bitfields,
                         std::vector<hrd_service>& expanded) {
  for (auto const& s : split(service, bitfields)) {
    expanded.emplace_back(new_service_from_split(s, service));
  }
}

}  // hrd
}  // loader
}  // motis
