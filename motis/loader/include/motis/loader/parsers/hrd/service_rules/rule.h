#pragma once

#include <cinttypes>
#include <map>
#include <vector>
#include <utility>

#include "motis/loader/parsers/hrd/service/hrd_service.h"
#include "motis/loader/bitfield.h"

namespace motis {
namespace loader {
namespace hrd {

struct resolved_rule_info {
  bitfield traffic_days;
  int eva_num_1, eva_num_2;
  uint8_t type;
};

typedef std::tuple<hrd_service*, hrd_service*, resolved_rule_info>
    service_combination;

struct rule {
  rule(bitfield const& mask) : mask_(mask) {}
  virtual ~rule() {}
  virtual int applies(hrd_service const& s) const = 0;
  virtual void add(hrd_service* s, int info) = 0;
  virtual std::vector<service_combination> service_combinations() const = 0;
  virtual resolved_rule_info rule_info() const = 0;

  bitfield const& mask_;

protected:
  std::vector<std::pair<int, uint64_t>> get_ids(hrd_service const&);
};

typedef std::pair<int, uint64_t> service_id;  // (train_num, admin)
typedef std::map<service_id, std::vector<std::shared_ptr<rule>>> rules;

}  // hrd
}  // loader
}  // motis
