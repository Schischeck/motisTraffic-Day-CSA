#include "motis/loader/parsers/hrd/service_rules/merge_split_rules_parser.h"

#include "parser/cstr.h"
#include "parser/arg_parser.h"

#include "motis/core/common/logging.h"
#include "motis/loader/util.h"
#include "motis/schedule-format/ServiceRules_generated.h"

using namespace parser;
using namespace motis::logging;

namespace motis {
namespace loader {
namespace hrd {

struct mss_rule : public rule {
  mss_rule(service_id id_1, service_id id_2, int eva_num_begin, int eva_num_end,
           bitfield const& mask)
      : id_1_(id_1),
        id_2_(id_2),
        eva_num_begin_(eva_num_begin),
        eva_num_end_(eva_num_end),
        mask_(mask) {}

  virtual ~mss_rule() {}

  int applies(hrd_service const& s) const override {
    // Check for non-empty intersection.
    if ((s.traffic_days_ & mask_).none()) {
      return false;
    }

    // Check if first and last stop of the common part are contained with the
    // correct service id.
    bool begin_found = false, end_found = false;
    for (unsigned section_idx = 0; section_idx < s.sections_.size();
         ++section_idx) {
      auto const& section = s.sections_[section_idx];
      auto const& from_stop = s.stops_[section_idx];
      auto const& to_stop = s.stops_[section_idx + 1];
      auto service_id = std::make_pair(section.train_num,
                                       raw_to_int<uint64_t>(section.admin));

      if (service_id != id_1_ && service_id != id_2_) {
        continue;
      }

      if (!end_found && from_stop.eva_num == eva_num_begin_) {
        begin_found = true;
      } else if (begin_found && to_stop.eva_num == eva_num_end_) {
        end_found = true;
      }
    }
    return begin_found && end_found;
  }

  void add(hrd_service* s, int /* info */) override {
    participants_.push_back(s);
  }

  std::vector<service_combination> service_combinations() const override {
    std::vector<service_combination> comb;
    for (auto const& s1 : participants_) {
      for (auto const& s2 : participants_) {
        if (s1 == s2) {
          continue;
        }

        auto intersection = s1->traffic_days_ & s2->traffic_days_ & mask_;
        if (intersection.any()) {
          comb.emplace_back(
              s1, s2, resolved_rule_info{intersection, eva_num_begin_,
                                         eva_num_end_, RuleType_MERGE_SPLIT});
        }
      }
    }
    return comb;
  }

  service_id id_1_, id_2_;
  int eva_num_begin_, eva_num_end_;
  bitfield const& mask_;
  std::vector<hrd_service*> participants_;
};

void parse_merge_split_service_rules(
    loaded_file const& src, std::map<int, bitfield> const& hrd_bitfields,
    rules& rules) {
  scoped_timer timer("parsing merge split rules");

  for_each_line_numbered(src.content, [&](cstr line, int line_number) {
    if (line.len < 53) {
      return;
    }

    auto it = hrd_bitfields.find(parse<int>(line.substr(47, size(6))));
    verify(it != std::end(hrd_bitfields), "missing bitfield: %s:%d", src.name,
           line_number);

    auto key_1 =
        std::make_pair(parse<int>(line.substr(18, size(5))),
                       raw_to_int<uint64_t>(line.substr(25, size(6)).trim()));
    auto key_2 =
        std::make_pair(parse<int>(line.substr(33, size(5))),
                       raw_to_int<uint64_t>(line.substr(40, size(6)).trim()));

    auto eva_num_begin = parse<int>(line.substr(0, size(7)));
    auto eva_num_end = parse<int>(line.substr(9, size(7)));
    std::shared_ptr<rule> rule(
        new mss_rule(key_1, key_2, eva_num_begin, eva_num_end, it->second));

    rules[key_1].push_back(rule);
    rules[key_2].push_back(rule);
  });
}

}  // hrd
}  // loader
}  // motis
