#pragma once

#include <string>

namespace motis {
namespace reliability {

namespace distributions_loader {
void load_distributions(std::string const root);

namespace detail {
void load_distributions_classes(std::string const file);
void load_distribution_mappings(std::string const file);
void load_distributions(std::string const file);
void load_start_distributions(std::string const file);
}  // namespace detail
}  // namespace distributions_loader

}  // namespace reliability
}  // namespace motis
