#include "motis/ris/ris.h"

#include "conf/date_time.h"
#include "conf/simple_config_param.h"

#include "motis/core/common/logging.h"

using namespace motis::module;

namespace motis {
namespace ris {

struct ris::impl {
  void init() {
    // TODO(felix)
    // parse_folder()
    // forward(new_time=init_time_)
  }

  msg_ptr upload(msg_ptr const&) {
    // TODO(felix)
    // write to folder()
    // parse_folder()
    // forward()
    return {};
  }

  msg_ptr forward(msg_ptr const&) {
    // TODO(felix)
    /*
      cursor c;
      std::vector<parser::buffer> blobs;
      for (auto const& m : c) {
        if (m->timstamp() > batch_to) {
          publish(blobs);
          blobs.clear();
        }
        blobs.emplace_back(m);
      }
    */
    return {};
  }

  std::string db_path_;
  std::string input_folder_;
  conf::holder<std::time_t> init_time_;
};

ris::ris() : module("RIS", "ris"), impl_(std::make_unique<impl>()) {
  string_param(impl_->db_path_, "ris_db", "db", "ris database path");
  string_param(impl_->input_folder_, "ris", "input_folder", "ris input folder");
  template_param(impl_->init_time_, {0l}, "init_time",
                 "'forward' the simulation clock (expects Unix timestamp)");
}

ris::~ris() = default;

void ris::init(motis::module::registry& r) {
  r.subscribe_void("/init", [this] { impl_->init(); });
  r.register_op("/ris/upload", [this](auto&& m) { return impl_->upload(m); });
  r.register_op("/ris/forward", [this](auto&& m) { return impl_->forward(m); });
}

}  // namespace ris
}  // namespace motis
