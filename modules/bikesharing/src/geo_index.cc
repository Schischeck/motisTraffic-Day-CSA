#include "motis/bikesharing/geo_index.h"

#include "boost/function_output_iterator.hpp"

#include "motis/core/common/geo.h"

using namespace motis::geo_detail;

namespace motis {
namespace bikesharing {

struct geo_index::impl {
  explicit impl(database const& db) {
    auto summary = db.get_summary();
    auto locations = summary.get()->terminals();

    std::vector<value> rtree_values;
    for (size_t i = 0; i < locations->size(); ++i) {
      auto location = locations->Get(i);
      rtree_values.push_back(
          std::make_pair(spherical_point(location->lng(), location->lat()), i));
      terminal_ids_.push_back(location->id()->str());
    }
    rtree_ = quadratic_rtree{rtree_values};
  }

  std::vector<close_terminal> get_terminals(double const lat, double const lng,
                                            double const radius) const {
    spherical_point loc(lng, lat);
    std::vector<close_terminal> vec;
    rtree_.query(
        bgi::intersects(generate_box(loc, radius)) &&
            bgi::satisfies([&loc, &radius](const value& v) {
              return distance_in_m(v.first, loc) < radius;
            }),
        boost::make_function_output_iterator([this, &loc, &vec](auto&& result) {
          vec.emplace_back(terminal_ids_[result.second],
                           distance_in_m(result.first, loc));
        }));
    return vec;
  };

  std::vector<std::string> terminal_ids_;
  quadratic_rtree rtree_;
};

geo_index::geo_index(database const& db) : impl_(new impl(db)) {}

geo_index::~geo_index() = default;

std::vector<close_terminal> geo_index::get_terminals(
    double const lat, double const lng, double const radius) const {
  return impl_->get_terminals(lat, lng, radius);
}

}  // namespace bikesharing
}  // namespace motis
