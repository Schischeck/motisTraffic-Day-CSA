#pragma once

#include <string>
#include <map>

#include "motis/loader/loaded_file.h"

namespace motis {
namespace loader {
namespace hrd {

struct provider_info {
  std::string short_name;
  std::string long_name;
  std::string full_name;
};

std::map<uint64_t, provider_info> parse_providers(loaded_file const&);

}  // loader
}  // motis
}  // hrd
