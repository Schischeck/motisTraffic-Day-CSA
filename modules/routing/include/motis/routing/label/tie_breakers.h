#pragma once

namespace motis {
namespace routing {

struct default_tb {
  template <typename Label>
  static bool dominates(bool, Label const&, Label const&) {
    return true;
  }
};

struct post_search_tb {
  template <typename Label>
  static bool dominates(bool could_dominate, Label const&, Label const&) {
    return could_dominate;
  }
};

}  // namespace routing
}  // namespace motis