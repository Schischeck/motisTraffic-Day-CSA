#include "motis/address/address.h"

namespace motis {
namespace address {

struct address::impl {};

address::address() : module("RailViz", "address") {}

address::~address() = default;

void address::init(motis::module::registry& reg) {
  namespace p = std::placeholders;
  // reg.register_op("/address", std::bind(&address::address, this, p::_1));
}

}  // namespace address
}  // namespace motis
