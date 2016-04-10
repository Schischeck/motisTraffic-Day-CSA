#pragma once

#include <cinttypes>
#include <string>

namespace motis {

struct provider {
  provider(std::string short_name, std::string long_name, std::string full_name)
      : short_name(std::move(short_name)), long_name(std::move(long_name)), full_name(std::move(full_name)){};

  std::string short_name;
  std::string long_name;
  std::string full_name;
};

}  // namespace motis
