#include "motis/ris/ris.h"

#include "motis/core/common/logging.h"

using namespace motis::module;

namespace motis {
namespace ris {

struct ris::impl {};

ris::ris() : module("RIS", "ris") {}

ris::~ris() = default;

void ris::init(motis::module::registry&) {}

}  // namespace ris
}  // namespace motis
