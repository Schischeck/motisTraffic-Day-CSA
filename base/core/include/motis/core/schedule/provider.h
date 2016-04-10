#pragma once

#include <cinttypes>
#include <string>

namespace motis {

struct provider {
  provider(std::string short_name, std::string long_name, std::string full_name)
      : short_name_(std::move(short_name)),
        long_name_(std::move(long_name)),
        full_name_(std::move(full_name)){};

  std::string short_name_;
  std::string long_name_;
  std::string full_name_;
};

}  // namespace motis
