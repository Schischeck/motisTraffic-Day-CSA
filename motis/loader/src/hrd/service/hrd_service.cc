#include "motis/loader/parsers/hrd/service/hrd_service.h"

#include <algorithm>

#include "parser/util.h"
#include "parser/arg_parser.h"

#include "motis/loader/util.h"
#include "motis/loader/parsers/hrd/service/range.h"

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
  sections.emplace_back(
      train_num.empty() ? last_section.train_num : parse<int>(train_num),
      admin.empty() ? last_section.admin : admin);

  return sections;
}

struct parse_information {
  int id_start, id_length;
  int from_eva_or_idx_start;
  int to_eva_or_idx_start;
  int from_hhmm_or_idx_start;
  int to_hhmm_or_idx_start;
} attribute_parse_info{3, 2, 6, 14, 29, 36},
  line_parse_info{3, 4, 9, 17, 25, 32},
  category_parse_info{3, 2, 7, 15, 23, 30};

std::vector<std::pair<cstr, range>> compute_ranges(
    std::vector<cstr> const& spec_lines,
    std::vector<hrd_service::stop> const& stops,
    parse_information const& parse_info) {
  std::vector<std::pair<cstr, range>> parsed(spec_lines.size());
  std::transform(begin(spec_lines), end(spec_lines), begin(parsed),
                 [&](cstr spec) {
    return std::make_pair(
        spec.substr(parse_info.id_start, size(parse_info.id_length)),
        range(stops, spec.substr(parse_info.from_eva_or_idx_start, size(7)),
              spec.substr(parse_info.to_eva_or_idx_start, size(7)),
              spec.substr(parse_info.from_hhmm_or_idx_start, size(6)),
              spec.substr(parse_info.to_hhmm_or_idx_start, size(6))));
  });
  return parsed;
}

/// Generic method to parse "ranged" information that needs to be written
/// to a section.
///
/// \param spec_lines  what to parse (lines parsed from the specification)
/// \param parse_info  indices to parse (where is the range / information)
/// \param stop        needed to compute ranges (eva numbers, event times)
/// \param sections    sections to fill with this information (where to write)
/// \param member      pointer to member to fill with this information
void parse_range(std::vector<cstr> const& spec_lines,
                 parse_information const& parse_info,
                 std::vector<hrd_service::stop> const& stops,
                 std::vector<hrd_service::section>& sections,
                 std::vector<cstr> hrd_service::section::*member) {
  for (auto const& r : compute_ranges(spec_lines, stops, parse_info)) {
    for (int i = r.second.from_idx; i < r.second.to_idx; ++i) {
      (sections[i].*member).push_back(r.first);
    }
  }
}

hrd_service::hrd_service(specification const& spec)
    : stops_(transform<decltype(stops_)>(begin(spec.stops), end(spec.stops),
                                         parse_stop)),
      sections_(std::accumulate(
          std::next(begin(spec.stops)),
          std::next(begin(spec.stops), spec.stops.size() - 1),
          std::vector<section>(
              {section(parse<int>(spec.internal_service.substr(3, size(5))),
                       spec.internal_service.substr(9, size(6)))}),
          parse_section)) {
  parse_range(spec.attributes, attribute_parse_info, stops_, sections_,
              &section::attribute_codes);
  parse_range(spec.categories, category_parse_info, stops_, sections_,
              &section::category);
  parse_range(spec.line_information, line_parse_info, stops_, sections_,
              &section::line_information);
  verify(valid(), "service invalid (line information / category)");
}

bool hrd_service::valid() const {
  for (auto const& section : sections_) {
    if (section.line_information.size() > 1 || section.category.size() != 1) {
      return false;
    }
  }
  return true;
}

}  // hrd
}  // loader
}  // motis
