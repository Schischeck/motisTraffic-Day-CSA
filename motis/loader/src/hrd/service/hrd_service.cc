#include "motis/loader/parsers/hrd/service/hrd_service.h"

#include "parser/arg_parser.h"

#include "motis/loader/util.h"

using namespace parser;

namespace motis {
namespace loader {
namespace hrd {

hrd_service::hrd_service(specification const& spec)
    : spec_(spec), sections_(spec.stops.size() - 1) {
  sections_[0] = {parse<int>(spec_.internal_service.substr(3, size(5))),
                  spec_.internal_service.substr(9, size(6))};

  eva_nums_.reserve(spec_.stops.size());
  events_.reserve(spec_.stops.size());

  int section_index = 0;
  for (auto const& stop : spec_.stops) {
    eva_nums_.push_back(parse<int>(stop.substr(0, size(7))));
    events_.push_back(
        {{hhmm_to_min(parse<int>(stop.substr(30, size(5)), NOT_SET)),
          stop[29] != '-'},
         {hhmm_to_min(parse<int>(stop.substr(37, size(5)), NOT_SET)),
          stop[36] != '-'}});

    if (events_.back().second.time == NOT_SET) {
      // There is no section starting at the last station.
      continue;
    }

    auto train_num = stop.substr(43, size(5)).trim();
    auto admin = stop.substr(43, size(6)).trim();

    auto& last_section = sections_.back();
    sections_[section_index] = {
        train_num.empty() ? last_section.train_num : parse<int>(train_num),
        admin.empty() ? last_section.admin : admin};
    ++section_index;
  }
}

}  // hrd
}  // loader
}  // motis
