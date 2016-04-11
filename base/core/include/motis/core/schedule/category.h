#pragma once

#include <cinttypes>
#include <string>

namespace motis {

struct category {
  enum output_rule {
    CATEGORY_AND_TRAIN_NUM,
    CATEGORY,
    TRAIN_NUM,
    NOTHING,
    PROVIDER_AND_TRAIN_NUM,
    PROVIDER,
    CATEGORY_AND_LINE,
    LINE
  };

  category(std::string name, uint8_t output_rule)
      : name_(std::move(name)), output_rule_(output_rule) {}

  std::string name_;
  uint8_t output_rule_;
};

}  // namespace motis
