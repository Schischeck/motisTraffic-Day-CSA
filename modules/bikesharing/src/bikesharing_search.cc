#include "motis/bikesharing/bikesharing_search.h"

#include <vector>

#include "motis/core/common/constants.h"
#include "motis/core/common/geo.h"
#include "motis/core/common/util.h"

using namespace motis::geo_detail;
using namespace motis::module;

namespace motis {
namespace bikesharing {

struct bike_departure {
  int walk_duration;
  int bike_duration;
  double bike_availability;

  persistable_terminal* from;
  persistable_terminal* to;
};

struct bikesharing_search::bikesharing_search_impl {
  using terminal_cache =
      std::map<std::string, std::unique_ptr<persistable_terminal>>;

  bikesharing_search_impl(database const& db) : db_(db) {
    auto summary = db.get_summary();
    auto locations = summary.get()->terminals();

    std::vector<value> rtree_values;
    for (size_t i = 0; i < locations->size(); ++i) {
      auto location = locations->Get(i);
      rtree_values.push_back({{location->lat(), location->lng()}, i});
      terminal_ids_.push_back(location->id()->str());
    }
    rtree_ = quadratic_rtree{rtree_values};
  }

  msg_ptr find_connections(BikesharingRequest const* req) {
    // right now only on departure side
    terminal_cache cache;

    std::multimap<std::string, bike_departure> departures;

    spherical_point dep_location(req->lng(), req->lat());
    for (auto it = rtree_.qbegin(
             bgi::intersects(generate_box(dep_location, MAX_BIKE_DIST)) &&
             bgi::satisfies([&dep_location](value const& v) {
               return distance_in_m(v.first, dep_location) < MAX_BIKE_DIST;
             }));
         it != rtree_.qend(); ++it) {

      int first_walk_dur = distance_in_m(it->first, dep_location) / WALK_SPEED;
      auto terminal = load_terminal(terminal_ids_[it->second], &cache);
      for (auto const& reachable_t : *terminal->get()->reachable()) {
        auto other_t = load_terminal(reachable_t->id()->str(), &cache);

        for (auto const& station : *other_t->get()->attached()) {
          auto bucket = timestamp_to_bucket(req->time() + first_walk_dur);
          double a = terminal->get()->availability()->Get(bucket)->minimum();

          bike_departure dep{first_walk_dur + station->duration(),
                             reachable_t->duration(), a, terminal, other_t};
          departures.emplace(station->id()->str(), dep);
        }
      }
    }

    return make_msg("");  // TODO
  }

  persistable_terminal* load_terminal(std::string const& id,
                                      terminal_cache* cache) {
    auto it = cache->find(id);
    if (it != cache->end()) {
      return it->second.get();
    }
    auto terminal_ptr = make_unique<persistable_terminal>(db_.get(id));
    return cache->emplace(id, std::move(terminal_ptr)).first->second.get();
  }

  database const& db_;
  std::vector<std::string> terminal_ids_;
  quadratic_rtree rtree_;
};

bikesharing_search::bikesharing_search(database const& db)
    : impl_(new bikesharing_search_impl(db)) {}
bikesharing_search::~bikesharing_search() = default;

msg_ptr bikesharing_search::find_connections(
    BikesharingRequest const* req) const {
  return impl_->find_connections(req);
}

}  // namespace bikesharing
}  // namespace motis
