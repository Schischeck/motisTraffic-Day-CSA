#include "motis/ris/ris.h"

#include "motis/core/common/logging.h"

using namespace motis::module;

namespace motis {
namespace ris {

struct ris::impl {
  impl(std::string db_path, std::string input_folder, time_t init_time) {}
};

ris::ris() : module("RIS", "ris") {
  string_param(db_path_, "ris_db", "db", "ris database path");
  string_param(input_folder_, "ris", "input_folder", "ris input folder");
  template_param(init_time_, {0l}, "init_time",
                 "'forward' the simulation clock (expects Unix timestamp)");
}

ris::~ris() = default;

void ris::init(motis::module::registry& r) {
  imp_ = std::make_unique<impl>(db_path_, input_folder_, init_time_);
  r.subscribe_void("/init", [this] { impl_->init_async(); });
}

}  // namespace ris
}  // namespace motis
