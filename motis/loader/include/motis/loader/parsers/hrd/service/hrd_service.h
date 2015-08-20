#pragma once

#include <vector>

#include "parser/cstr.h"

#include "motis/loader/parsers/hrd/service/specification.h"

namespace motis {
namespace loader {
namespace hrd {

struct hrd_service {

  static const int NOT_SET;

  struct event {
    int time;
    bool in_out_allowed;
  };

  struct stop {
    int eva_num;
    event arr, dep;
  };

  struct attribute {
    attribute(int bitfield_num, parser::cstr code)
        : bitfield_num(bitfield_num), code(code) {}

    int bitfield_num;
    parser::cstr code;

    friend bool operator==(attribute const& lhs, attribute const& rhs) {
      return lhs.bitfield_num == rhs.bitfield_num && lhs.code == rhs.code;
    }
  };

  struct section {
    section() = default;
    section(int train_num, parser::cstr admin)
        : train_num(train_num), admin(admin) {}

    int train_num;
    parser::cstr admin;
    std::vector<attribute> attributes;
    std::vector<parser::cstr> category;
    std::vector<parser::cstr> line_information;
    std::vector<int> traffic_days;
  };

  hrd_service(specification const& spec);

  bool valid() const;

  std::vector<stop> stops_;
  std::vector<section> sections_;
};
}  // hrd
}  // loader
}  // motis
