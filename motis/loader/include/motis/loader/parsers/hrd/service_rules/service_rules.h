#pragma once

#include <memory>

#include "motis/loader/util.h"

#include "motis/loader/parsers/hrd/service/hrd_service.h"
#include "motis/loader/parsers/hrd/service_rules/rule.h"

namespace motis {
namespace loader {
namespace hrd {

struct service_rules {
  service_rules(rules rules) : rules_(std::move(rules)) {}

  bool add_service(hrd_service& s) {
    bool applied = false;
    for (auto const& id : s.get_ids()) {
      auto it = rules_.find(id);
      if (it != end(rules_)) {
        applied = applied || try_apply_rules(s, it->second);
      }
    }
    return applied;
  }

  bool try_apply_rules(hrd_service& s,
                       std::vector<std::shared_ptr<rule>>& rules) {
    bool applied = false;
    for (auto& r : rules) {
      int info = r->applies(s);
      if (info) {
        services_.emplace_back(std::make_shared<hrd_service>(s));
        r->add(*services_.back(), info);
        applied = true;
      }
    }
    return applied;
  }

  rules rules_;
  std::vector<std::shared_ptr<hrd_service>> services_;
};

}  // hrd
}  // loader
}  // motis
