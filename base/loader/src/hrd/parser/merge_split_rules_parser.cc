#include "motis/loader/hrd/parser/merge_split_rules_parser.h"

#include <set>
#include <vector>

#include "parser/cstr.h"
#include "parser/arg_parser.h"

#include "motis/core/common/logging.h"
#include "motis/loader/util.h"
#include "motis/schedule-format/RuleService_generated.h"

using namespace parser;
using namespace motis::logging;

namespace motis {
namespace loader {
namespace hrd {

struct mss_rule : public service_rule {
  mss_rule(service_id id_1, service_id id_2, int eva_num_begin, int eva_num_end,
           bitfield const& mask)
      : service_rule(mask),
        id_1_(id_1),
        id_2_(id_2),
        eva_num_begin_(eva_num_begin),
        eva_num_end_(eva_num_end) {}

  virtual ~mss_rule() {}

  int applies(hrd_service const& s) const override {
    // Check for non-empty intersection.
    if ((s.traffic_days_ & mask_).none()) {
      return false;
    }

    // Check if first and last stop of the common part are contained with the
    // correct service id.
    bool begin_found = false, end_found = false;
    for (unsigned section_idx = 0;
         section_idx < s.sections_.size() && !(begin_found && end_found);
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
      }
      if (begin_found && to_stop.eva_num == eva_num_end_) {
        end_found = true;
      }
    }
    return begin_found && end_found;
  }

  void add(hrd_service* s, int /* info */) override {
    participants_.push_back(s);
  }

  std::pair<int, int> get_event_time(
      hrd_service const* s, int eva_num,
      hrd_service::event hrd_service::stop::*ev) const {
    auto first_stop_it = std::find_if(
        begin(s->stops_), end(s->stops_),
        [&](hrd_service::stop const& st) { return st.eva_num == eva_num; });
    assert(first_stop_it != end(s->stops_));
    return std::make_pair(((*first_stop_it).*ev).time,
                          std::distance(begin(s->stops_), first_stop_it));
  }

  bool all_ms_events_exist(hrd_service const* s1, hrd_service const* s2,
                           int mss_begin, int mss_end) const {
    int merge_time_s1, merge_time_s2, merge_idx_s1, merge_idx_s2;
    std::tie(merge_time_s1, merge_idx_s1) =
        get_event_time(s1, mss_begin, &hrd_service::stop::dep);
    std::tie(merge_time_s2, merge_idx_s2) =
        get_event_time(s2, mss_begin, &hrd_service::stop::dep);

    if (merge_time_s1 != merge_time_s2) {
      return false;
    }

    int split_time_s1, split_time_s2, split_idx_s1, split_idx_s2;
    std::tie(split_time_s1, split_idx_s1) =
        get_event_time(s1, mss_end, &hrd_service::stop::arr);
    std::tie(split_time_s2, split_idx_s2) =
        get_event_time(s2, mss_end, &hrd_service::stop::arr);

    if (split_time_s1 != split_time_s2) {
      return false;
    }

    auto const ms_distance_s1 = split_idx_s1 - merge_idx_s1;
    auto const ms_distance_s2 = split_idx_s2 - merge_idx_s2;
    if (ms_distance_s1 != ms_distance_s2) {
      return false;
    }

    // ensure that all stops between the merge and split match
    --split_idx_s1;
    --split_idx_s2;
    while (++merge_idx_s1 <= split_idx_s1 && ++merge_idx_s2 <= split_idx_s2) {
      auto const& stop_s1 = s1->stops_[split_idx_s1];
      auto const& stop_s2 = s2->stops_[split_idx_s2];
      if (stop_s1.eva_num != stop_s2.eva_num ||
          stop_s1.arr.time != stop_s2.arr.time ||
          stop_s1.dep.time != stop_s2.dep.time) {
        return false;
      }
    }
    return true;
  }

  std::vector<service_combination> service_combinations() const override {
    std::vector<service_combination> unordered_pairs;
    std::set<std::pair<hrd_service*, hrd_service*>> combinations;
    for (auto s1 : participants_) {
      for (auto s2 : participants_) {
        if (s1 == s2 ||
            combinations.find(std::make_pair(s2, s1)) != end(combinations)) {
          continue;
        }
        combinations.emplace(s1, s2);

        auto intersection = s1->traffic_days_ & s2->traffic_days_ & mask_;
        if (intersection.any() &&
            all_ms_events_exist(s1, s2, eva_num_begin_, eva_num_end_)) {
          unordered_pairs.emplace_back(
              s1, s2, resolved_rule_info{intersection, eva_num_begin_,
                                         eva_num_end_, RuleType_MERGE_SPLIT});
        }
      }
    }
    return unordered_pairs;
  }

  resolved_rule_info rule_info() const override {
    return resolved_rule_info{mask_, eva_num_begin_, eva_num_end_,
                              RuleType_MERGE_SPLIT};
  }

  service_id id_1_, id_2_;
  int eva_num_begin_, eva_num_end_;
  std::vector<hrd_service*> participants_;
};

void parse_merge_split_service_rules(
    loaded_file const& file, std::map<int, bitfield> const& hrd_bitfields,
    service_rules& rules) {
  scoped_timer timer("parsing merge split rules");

  for_each_line_numbered(file.content(), [&](cstr line, int line_number) {
    if (line.len < 53) {
      return;
    }

    auto it = hrd_bitfields.find(parse<int>(line.substr(47, size(6))));
    verify(it != std::end(hrd_bitfields), "missing bitfield: %s:%d",
           file.name(), line_number);

    auto key_1 =
        std::make_pair(parse<int>(line.substr(18, size(5))),
                       raw_to_int<uint64_t>(line.substr(25, size(6)).trim()));
    auto key_2 =
        std::make_pair(parse<int>(line.substr(33, size(5))),
                       raw_to_int<uint64_t>(line.substr(40, size(6)).trim()));

    auto eva_num_begin = parse<int>(line.substr(0, size(7)));
    auto eva_num_end = parse<int>(line.substr(9, size(7)));
    std::shared_ptr<service_rule> rule(
        new mss_rule(key_1, key_2, eva_num_begin, eva_num_end, it->second));

    rules[key_1].push_back(rule);
    rules[key_2].push_back(rule);
  });
}

}  // hrd
}  // loader
}  // motis
