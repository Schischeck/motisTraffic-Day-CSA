#pragma once

#include <map>
#include <vector>
#include <utility>

#include "motis/loader/parsers/hrd/service/hrd_service.h"
#include "motis/loader/bitfield.h"

namespace motis {
namespace loader {
namespace hrd {

typedef std::pair<int, uint64_t> service_id;  // (train_num, admin)
struct rule;
typedef std::map<service_id, std::vector<std::shared_ptr<rule>>> rules;

struct rule {
  virtual ~rule() {}
  virtual int applies(hrd_service& s) const = 0;
  virtual void add(hrd_service& s, int info) = 0;
  virtual std::vector<std::pair<hrd_service*, hrd_service*>>
  service_combinations() const = 0;

protected:
  std::vector<std::pair<int, uint64_t>> get_ids(hrd_service const&);
};

}  // hrd
}  // loader
}  // motis
