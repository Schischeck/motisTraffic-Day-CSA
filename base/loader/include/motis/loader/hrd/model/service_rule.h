#pragma once

#include <cinttypes>
#include <map>
#include <utility>
#include <vector>

#include "motis/loader/bitfield.h"
#include "motis/loader/hrd/model/hrd_service.h"

namespace motis {
namespace loader {
namespace hrd {

struct resolved_rule_info {
  bitfield traffic_days_;
  int eva_num_1_, eva_num_2_;
  uint8_t type_;
};

typedef std::tuple<hrd_service*, hrd_service*, resolved_rule_info>
    service_combination;

struct service_rule {
  explicit service_rule(bitfield const& mask) : mask_(mask) {}
  virtual ~service_rule() = default;
  virtual int applies(hrd_service const&) const = 0;
  virtual void add(hrd_service*, int info) = 0;
  virtual std::vector<service_combination> service_combinations() const = 0;
  virtual resolved_rule_info rule_info() const = 0;

  bitfield const& mask_;

protected:
  std::vector<std::pair<int, uint64_t>> get_ids(hrd_service const&);
};

typedef std::pair<int, uint64_t> service_id;  // (train_num, admin)
typedef std::map<service_id, std::vector<std::shared_ptr<service_rule>>>
    service_rules;

}  // namespace hrd
}  // namespace loader
}  // namespace motis
