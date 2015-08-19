#include "motis/loader/parsers/hrd/service/hrd_service.h"

#include <algorithm>

#include "parser/arg_parser.h"

#include "motis/loader/util.h"

using namespace parser;

namespace motis {
namespace loader {
namespace hrd {

const int hrd_service::NOT_SET = -1;

hrd_service::stop parse_stop(cstr stop) {
  return {
      parse<int>(stop.substr(0, size(7))),
      {hhmm_to_min(parse<int>(stop.substr(30, size(5)), hrd_service::NOT_SET)),
       stop[29] != '-'},
      {hhmm_to_min(parse<int>(stop.substr(37, size(5)), hrd_service::NOT_SET)),
       stop[36] != '-'}};
}

std::vector<hrd_service::section> parse_section(
    std::vector<hrd_service::section>& sections, cstr stop) {
  auto train_num = stop.substr(43, size(5)).trim();
  auto admin = stop.substr(43, size(6)).trim();

  auto& last_section = sections.back();
  sections.push_back(
      {train_num.empty() ? last_section.train_num : parse<int>(train_num),
       admin.empty() ? last_section.admin : admin});

  return sections;
}

hrd_service::hrd_service(specification const& spec)
    : stops_(transform<decltype(stops_)>(begin(spec.stops), end(spec.stops),
                                         parse_stop)),
      sections_(std::accumulate(
          std::next(begin(spec.stops)),
          std::next(begin(spec.stops), spec.stops.size() - 1),
          std::vector<section>(
              {{parse<int>(spec.internal_service.substr(3, size(5))),
                spec.internal_service.substr(9, size(6))}}),
          parse_section)) {}

}  // hrd
}  // loader
}  // motis
