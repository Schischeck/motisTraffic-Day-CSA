#pragma once

namespace motis {
namespace routing {

struct no_intercity_filter {
  template <typename Label>
  static bool is_filtered(Label const& l) {
    if(l.connection_ != nullptr) {
      return l.connection_->full_con_->clasz_ < 3;     
    } else {
      return false;
    }
  }
};

}  // namespace routing
}  // namespace motis
