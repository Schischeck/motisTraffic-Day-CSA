#include "motis/loader/hrd/parser/through_services_parser.h"

#include "parser/arg_parser.h"
#include "parser/cstr.h"

#include "motis/core/common/logging.h"
#include "motis/loader/util.h"
#include "motis/schedule-format/RuleService_generated.h"

using namespace parser;
using namespace motis::logging;

namespace motis {
namespace loader {
namespace hrd {

struct ts_rule : public service_rule {
  ts_rule(service_id id_1, service_id id_2, int eva_num, bitfield const& mask)
      : service_rule(mask),
        id_1_(std::move(id_1)),
        id_2_(std::move(id_2)),
        eva_num_(eva_num) {}

  ~ts_rule() override = default;

  ts_rule(ts_rule const&) = delete;
  ts_rule(ts_rule&&) = delete;
  ts_rule& operator=(ts_rule const&) = delete;
  ts_rule& operator=(ts_rule&&) = delete;

  int applies(hrd_service const& s) const override {
    // Check for non-empty intersection.
    if ((s.traffic_days_ & mask_).none()) {
      return 0;
    }

    // Assuming s is service (1): Check last stop.
    auto last_stop = s.stops_.back();
    auto last_section = s.sections_.back();
    if (last_stop.eva_num_ == eva_num_ &&
        last_section.train_num_ == id_1_.first &&
        raw_to_int<uint64_t>(last_section.admin_) == id_1_.second) {
      return 1;
    }

    // Assuming s is service (2).
    for (unsigned section_idx = 0; section_idx < s.sections_.size();
         ++section_idx) {
      auto from_stop = s.stops_[section_idx];
      auto section = s.sections_[section_idx];

      if (from_stop.eva_num_ == eva_num_ && section.train_num_ == id_2_.first &&
          raw_to_int<uint64_t>(section.admin_) == id_2_.second) {
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
  std::vector<hrd_service*> participants_1_;
  std::vector<hrd_service*> participants_2_;
};

template <typename T>
void parse_through_service_rules(loaded_file const& file,
                                 std::map<int, bitfield> const& hrd_bitfields,
                                 service_rules& rules, T const& config) {
  scoped_timer timer("parsing through trains");
  for_each_line_numbered(file.content(), [&](cstr line, int line_number) {
    if (line.len < 40) {
      return;
    }

    auto it = hrd_bitfields.find(
        parse<int>(parse_field(line, config.through_services.bitfield)));
    verify(it != std::end(hrd_bitfields), "missing bitfield: %s:%d",
           file.name(), line_number);

    auto key_1 = std::make_pair(
        parse<int>(parse_field(line, config.through_services.key1_nr)),
        raw_to_int<uint64_t>(
            parse_field(line, config.through_services.key1_admin)));
    auto key_2 = std::make_pair(
        parse<int>(parse_field(line, config.through_services.key2_nr)),
        raw_to_int<uint64_t>(
            parse_field(line, config.through_services.key2_admin)));
    std::shared_ptr<service_rule> rule(
        new ts_rule(key_1, key_2,
                    parse<int>(parse_field(line, config.through_services.eva)),
                    it->second));

    rules[key_1].push_back(rule);
    rules[key_2].push_back(rule);
  });
}

}  // namespace hrd
}  // namespace loader
}  // namespace motis
