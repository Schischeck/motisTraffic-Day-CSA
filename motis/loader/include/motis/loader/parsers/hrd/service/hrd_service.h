#pragma once

#include <vector>

#include "parser/cstr.h"

#include "motis/loader/parsers/hrd/service/specification.h"

namespace motis {
namespace loader {
namespace hrd {

struct hrd_service {

  static constexpr int NOT_SET = -1;

  struct event {
    int time;
    bool in_out_allowed;
  };

  struct section {
    int train_num;
    parser::cstr admin;
  };

  hrd_service(specification const& spec);

  specification const& spec_;
  std::vector<int> eva_nums_;
  std::vector<std::pair<event, event>> events_;
  std::vector<section> sections_;
};
}  // hrd
}  // loader
}  // motis
