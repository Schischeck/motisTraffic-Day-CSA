#pragma once

#include <vector>

#include "parser/cstr.h"

#include "motis/loader/util.h"

#include "motis/loader/parsers/hrd/bitfields_parser.h"

namespace motis {
namespace loader {
namespace hrd {

struct parser_info {
  char const* filename;
  int line_number;
};

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

  enum direction_type { EVA_NUMBER, DIRECTION_CODE };
  struct section {
    section() = default;
    section(int train_num, parser::cstr admin)
        : train_num(train_num), admin(admin) {}

    int train_num;
    parser::cstr admin;
    std::vector<attribute> attributes;
    std::vector<parser::cstr> category;
    std::vector<parser::cstr> line_information;
    std::vector<std::pair<uint64_t, int>> directions;
    std::vector<int> traffic_days;
  };

  hrd_service(parser_info origin, int num_repetitions, int interval,
              std::vector<stop> stops, std::vector<section> sections,
              bitfield traffic_days)
      : origin_(std::move(origin)),
        num_repetitions_(num_repetitions),
        interval_(interval),
        stops_(std::move(stops)),
        sections_(std::move(sections)),
        traffic_days_(std::move(traffic_days)) {}

  explicit hrd_service(specification const& spec);

  void verify_service() const;

  std::vector<std::pair<int, uint64_t>> get_ids() const {
    std::vector<std::pair<int, uint64_t>> ids;

    // Add first service id.
    auto const& first_section = sections_.front();
    ids.push_back(std::make_pair(first_section.train_num,
                                 raw_to_int<uint64_t>(first_section.admin)));

    // Add new service id if it changed.
    for (size_t i = 1; i < sections_.size(); ++i) {
      auto id = std::make_pair(sections_[i].train_num,
                               raw_to_int<uint64_t>(sections_[i].admin));
      if (id != ids.back()) {
        ids.push_back(id);
      }
    }

    return ids;
  }

  parser_info origin_;
  int num_repetitions_;
  int interval_;
  std::vector<stop> stops_;
  std::vector<section> sections_;
  bitfield traffic_days_;
};

}  // hrd
}  // loader
}  // motis
