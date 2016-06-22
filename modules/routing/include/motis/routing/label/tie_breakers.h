#pragma once

namespace motis {
namespace routing {

struct default_tb {
  template <typename Label>
  static bool dominates(bool, Label const& a, Label const& b) {
    return a.absurdity_ <= b.absurdity_;
  }
};

struct post_search_tb {
  template <typename Label>
  static bool dominates(bool could_dominate, Label const& a, Label const& b) {
    return could_dominate || a.absurdity_ <= b.absurdity_;
  }
};

}  // namespace routing
}  // namespace motis
