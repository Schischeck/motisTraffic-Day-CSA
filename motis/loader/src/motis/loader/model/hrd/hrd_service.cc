#include "motis/loader/model/hrd/hrd_service.h"

#include <tuple>
#include <algorithm>
#include <numeric>

#include "parser/util.h"
#include "parser/arg_parser.h"

#include "motis/loader/util.h"
#include "motis/loader/model/hrd/range.h"

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
  auto admin = stop.substr(49, size(6)).trim();

  auto last_section = sections.back();
  sections.emplace_back(
      train_num.empty() ? last_section.train_num : parse<int>(train_num),
      admin.empty() ? last_section.admin : admin);

  return sections;
}

struct range_parse_information {
  int from_eva_or_idx_start;
  int to_eva_or_idx_start;
  int from_hhmm_or_idx_start;
  int to_hhmm_or_idx_start;
} attribute_parse_info{6, 14, 29, 36}, line_parse_info{9, 17, 25, 32},
    category_parse_info{7, 15, 23, 30}, traffic_days_parse_info{6, 14, 29, 36},
    direction_parse_info{13, 21, 29, 36};

std::vector<std::pair<cstr, range>> compute_ranges(
    std::vector<cstr> const& spec_lines,
    std::vector<hrd_service::stop> const& stops,
    range_parse_information const& parse_info) {
  std::vector<std::pair<cstr, range>> parsed(spec_lines.size());
  std::transform(
      begin(spec_lines), end(spec_lines), begin(parsed), [&](cstr spec) {
        return std::make_pair(
            spec,
            range(stops, spec.substr(parse_info.from_eva_or_idx_start, size(7)),
                  spec.substr(parse_info.to_eva_or_idx_start, size(7)),
                  spec.substr(parse_info.from_hhmm_or_idx_start, size(6)),
                  spec.substr(parse_info.to_hhmm_or_idx_start, size(6))));
      });
  return parsed;
}

template <typename TargetInformationType, typename TargetInformationParserFun>
void parse_range(
    std::vector<cstr> const& spec_lines,
    range_parse_information const& parse_info,
    std::vector<hrd_service::stop> const& stops,
    std::vector<hrd_service::section>& sections,
    std::vector<TargetInformationType> hrd_service::section::*member,
    TargetInformationParserFun parse_target_info) {
  for (auto const& r : compute_ranges(spec_lines, stops, parse_info)) {
    TargetInformationType target_info = parse_target_info(r.first, r.second);
    for (int i = r.second.from_idx; i < r.second.to_idx; ++i) {
      (sections[i].*member).push_back(target_info);
    }
  }
}

hrd_service::hrd_service(specification const& spec)
    : origin_(parser_info{spec.filename_, spec.line_number_}),
      num_repetitions_(parse<int>(spec.internal_service.substr(22, size(3)))),
      interval_(parse<int>(spec.internal_service.substr(26, size(3)))),
      stops_(transform<decltype(stops_)>(begin(spec.stops), end(spec.stops),
                                         parse_stop)),
      sections_(std::accumulate(
          std::next(begin(spec.stops)),
          std::next(begin(spec.stops), spec.stops.size() - 1),
          std::vector<section>(
              {section(parse<int>(spec.internal_service.substr(3, size(5))),
                       spec.internal_service.substr(9, size(6)))}),
          parse_section)) {
  parse_range(spec.attributes, attribute_parse_info, stops_, sections_,
              &section::attributes, [](cstr line, range const&) {
                return attribute{parse<int>(line.substr(22, size(6))),
                                 line.substr(3, size(2))};
              });

  parse_range(spec.categories, category_parse_info, stops_, sections_,
              &section::category,
              [](cstr line, range const&) { return line.substr(3, size(3)); });

  parse_range(spec.line_information, line_parse_info, stops_, sections_,
              &section::line_information, [](cstr line, range const&) {
                return line.substr(3, size(5)).trim();
              });

  parse_range(spec.traffic_days, traffic_days_parse_info, stops_, sections_,
              &section::traffic_days, [](cstr line, range const&) {
                return parse<int>(line.substr(22, size(6)));
              });

  parse_range(
      spec.directions, direction_parse_info, stops_, sections_,
      &section::directions, [&](cstr line, range const& r) {
        if (isdigit(line[5])) {
          return std::make_pair(parse<uint64_t>(line.substr(5, size(7))),
                                EVA_NUMBER);
        } else if (line[5] == ' ') {
          return std::make_pair(static_cast<uint64_t>(stops_[r.to_idx].eva_num),
                                EVA_NUMBER);
        } else {
          return std::make_pair(raw_to_int<uint64_t>(line.substr(5, size(7))),
                                DIRECTION_CODE);
        }
      });

  verify_service();
}

void hrd_service::verify_service() const {
  int section_index = 0;
  verify(stops_.size() >= 2, "service with less than 2 stops");
  for (auto const& section : sections_) {
    verify(section.traffic_days.size() == 1,
           "section %d invalid: %lu multiple traffic days", section_index,
           section.traffic_days.size());
    verify(section.line_information.size() <= 1,
           "section %d invalid: %lu line information", section_index,
           section.line_information.size());
    verify(section.category.size() == 1, "section %d invalid: %lu categories",
           section_index, section.category.size());
    verify(section.directions.size() <= 1,
           "section %d invalid: %lu direction information", section_index,
           section.directions.size());
    ++section_index;
  }
}

}  // hrd
}  // loader
}  // motis
