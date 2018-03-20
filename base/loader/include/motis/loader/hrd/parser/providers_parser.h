#pragma once

#include <map>
#include <string>

#include "motis/loader/hrd/parse_config.h"
#include "motis/loader/loaded_file.h"

namespace motis {
namespace loader {
namespace hrd {

struct provider_info {
  std::string short_name_;
  std::string long_name_;
  std::string full_name_;
};

std::map<uint64_t, provider_info> parse_providers(loaded_file const&,
                                                  config const&);

}  // namespace hrd
}  // namespace loader
}  // namespace motis
