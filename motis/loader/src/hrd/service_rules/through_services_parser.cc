#include "motis/loader/parsers/hrd/service_rules/through_services_parser.h"

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

struct ts_rule : public rule {
  ts_rule(service_id id_1, service_id id_2, int eva_num, bitfield const& mask)
      : id_1_(id_1), id_2_(id_2), eva_num_(eva_num), mask_(mask) {}

  virtual ~ts_rule() {}

  int applies(hrd_service const& s) const override {
    // Check for non-empty intersection.
    if ((s.traffic_days_ & mask_).none()) {
      return false;
    }

    // Assuming s is service (1): Check last stop.
    auto last_stop = s.stops_.back();
    auto last_section = s.sections_.back();
    if (last_stop.eva_num == eva_num_ &&
        last_section.train_num == id_1_.first &&
        raw_to_int<uint64_t>(last_section.admin) == id_1_.second) {
      return 1;
    }

    // Assuming s is service (2).
    for (unsigned section_idx = 0; section_idx < s.sections_.size();
         ++section_idx) {
      auto from_stop = s.stops_[section_idx];
      auto section = s.sections_[section_idx];

      if (from_stop.eva_num == eva_num_ && section.train_num == id_2_.first &&
          raw_to_int<uint64_t>(section.admin) == id_2_.second) {
        return 2;
      }
    }

    // No match.
    return 0;
  }

  void add(hrd_service* s, int info) override {
    if (info == 1) {
      participants_1_.push_back(s);
    } else {
      participants_2_.push_back(s);
    }
  }

  std::vector<service_combination> service_combinations() const override {
    std::vector<service_combination> comb;
    for (auto const& s1 : participants_1_) {
      for (auto const& s2 : participants_2_) {
        auto intersection = s1->traffic_days_ & s2->traffic_days_ & mask_;
        if (intersection.any()) {
          comb.emplace_back(s1, s2,
                            resolved_rule_info{intersection, eva_num_, eva_num_,
                                               RuleType_THROUGH});
        }
      }
    }
    return comb;
  }

  resolved_rule_info rule_info() const override {
    return resolved_rule_info{mask_, eva_num_, eva_num_, RuleType_THROUGH};
  }

  service_id id_1_, id_2_;
  int eva_num_;
  bitfield const& mask_;
  std::vector<hrd_service*> participants_1_;
  std::vector<hrd_service*> participants_2_;
};

void parse_through_service_rules(loaded_file const& src,
                                 std::map<int, bitfield> const& hrd_bitfields,
                                 rules& rules) {
  scoped_timer timer("parsing through trains");
  for_each_line_numbered(src.content, [&](cstr line, int line_number) {
    if (line.len < 40) {
      return;
    }

    auto it = hrd_bitfields.find(parse<int>(line.substr(34, size(6))));
    verify(it != std::end(hrd_bitfields), "missing bitfield: %s:%d", src.name,
           line_number);

    auto key_1 = std::make_pair(parse<int>(line.substr(0, size(5))),
                                raw_to_int<uint64_t>(line.substr(6, size(6))));
    auto key_2 = std::make_pair(parse<int>(line.substr(21, size(5))),
                                raw_to_int<uint64_t>(line.substr(27, size(6))));
    std::shared_ptr<rule> rule(new ts_rule(
        key_1, key_2, parse<int>(line.substr(13, size(7))), it->second));

    rules[key_1].push_back(rule);
    rules[key_2].push_back(rule);
  });
}

}  // hrd
}  // loader
}  // motis
