#pragma once

#include <cinttypes>
#include <string>

namespace motis {

struct provider {
  provider(std::string short_name, std::string long_name, std::string full_name)
      : short_name(short_name), long_name(long_name), full_name(full_name){};

  std::string short_name;
  std::string long_name;
  std::string full_name;
};

}  // namespace motis
