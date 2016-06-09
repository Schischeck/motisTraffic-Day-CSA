#include "motis/bikesharing/search.h"

#include <vector>

#include "motis/core/common/constants.h"
#include "motis/core/common/geo.h"
#include "motis/core/common/get_or_create.h"
#include "motis/core/common/util.h"

using namespace flatbuffers;
using namespace motis::geo_detail;
using namespace motis::module;

namespace motis {
namespace bikesharing {

constexpr uint64_t kSecondsPerHour = 3600;

using availability_bucket = BikesharingAvailability;
struct bikesharing_search::impl {
  struct bike_edge {
    int walk_duration_;
    int bike_duration_;
    std::vector<availability_bucket> availability_;

    persistable_terminal* from_;
    persistable_terminal* to_;

    std::string eva_nr_;
  };

  struct context {
    message_creator b_;
    std::map<std::string, std::unique_ptr<persistable_terminal>> terminals_;
    std::map<std::string, Offset<BikesharingTerminal>> terminal_offsets_;
  };

  explicit impl(database const& db) : db_(db) {
    auto summary = db.get_summary();
    auto locations = summary.get()->terminals();

    std::vector<value> rtree_values;
    for (size_t i = 0; i < locations->size(); ++i) {
      auto location = locations->Get(i);
      rtree_values.push_back(
          std::make_pair(spherical_point(location->lat(), location->lng()), i));
      terminal_ids_.push_back(location->id()->str());
    }
    rtree_ = quadratic_rtree{rtree_values};
  }

  msg_ptr find_connections(BikesharingRequest const* req) const {
    context ctx;
    auto const edges = req->type() == Type::Type_Departure
                           ? find_departures(ctx, req)
                           : find_arrivals(ctx, req);
    ctx.b_.create_and_finish(MsgContent_BikesharingResponse,
                             CreateBikesharingResponse(ctx.b_, edges).Union());
    return make_msg(ctx.b_);
  }

  Offset<Vector<Offset<BikesharingEdge>>> find_departures(
      context& ctx, BikesharingRequest const* req) const {
    auto begin =
        req->interval()->begin() - req->interval()->begin() % kSecondsPerHour;
    auto end = req->interval()->end();
    auto first_bucket = timestamp_to_bucket(begin);

    std::multimap<std::string, bike_edge> departures;
    foreach_terminal_in_walk_dist(
        req->position()->lng(), req->position()->lat(),
        [&, this](std::string const& id, int walk_dur) {
          auto const& from_t = load_terminal(ctx, id);
          for (auto const& reachable_t : *from_t->get()->reachable()) {
            auto to_t = load_terminal(ctx, reachable_t->id()->str());

            for (auto const& station : *to_t->get()->attached()) {
              auto availability =
                  get_availability(from_t->get(), begin, end, first_bucket,
                                   req->availability_aggregator());
              bike_edge edge{walk_dur + station->duration(),
                             reachable_t->duration(),
                             availability,
                             from_t,
                             to_t,
                             station->id()->str()};
              departures.emplace(id, edge);
            }
          }
        });

    return serialize_edges(ctx, departures);
  }

  Offset<Vector<Offset<BikesharingEdge>>> find_arrivals(
      context& ctx, BikesharingRequest const* req) const {
    auto begin =
        req->interval()->begin() - req->interval()->begin() % kSecondsPerHour;
    auto end = req->interval()->end() + MAX_TRAVEL_TIME_SECONDS;
    auto first_bucket = timestamp_to_bucket(begin);

    std::multimap<std::string, bike_edge> arrivals;
    foreach_terminal_in_walk_dist(
        req->position()->lng(), req->position()->lat(),
        [&, this](std::string const& id, int walk_dur) {
          auto const& to_t = load_terminal(ctx, id);
          for (auto const& reachable_t : *to_t->get()->reachable()) {
            auto from_t = load_terminal(ctx, reachable_t->id()->str());

            for (auto const& station : *from_t->get()->attached()) {
              auto availability =
                  get_availability(from_t->get(), begin, end, first_bucket,
                                   req->availability_aggregator());
              bike_edge edge{walk_dur + station->duration(),
                             reachable_t->duration(),
                             availability,
                             from_t,
                             to_t,
                             station->id()->str()};
              arrivals.emplace(id, edge);
            }
          }
        });

    return serialize_edges(ctx, arrivals);
  }

  template <typename F>
  void foreach_terminal_in_walk_dist(double lat, double lng, F func) const {
    spherical_point loc(lng, lat);

    std::vector<value> result_n;
    rtree_.query(bgi::intersects(generate_box(loc, MAX_WALK_DIST)) &&
                     bgi::satisfies([&loc](const value& v) {
                       return distance_in_m(v.first, loc) < MAX_WALK_DIST;
                     }),
                 std::back_inserter(result_n));

    for (const auto& result : result_n) {
      // TODO(Sebastian Fahnenschreiber) OSRM
      func(terminal_ids_[result.second],
           distance_in_m(result.first, loc) * LINEAR_DIST_APPROX / WALK_SPEED);
    }
  }

  persistable_terminal* load_terminal(context& c, std::string const& id) const {
    return get_or_create(
               c.terminals_, id,
               [&]() {
                 return std::make_unique<persistable_terminal>(db_.get(id));
               })
        .get();
  }

  std::vector<availability_bucket> get_availability(
      Terminal const* term, uint64_t begin, uint64_t end, size_t bucket,
      AvailabilityAggregator aggr) const {
    std::vector<availability_bucket> availability;
    for (auto t = begin; t < end; t += kSecondsPerHour) {
      double val = get_availability(term->availability()->Get(bucket++), aggr);
      availability.push_back({t, t + kSecondsPerHour, val});
    }
    return availability;
  }

  double get_availability(Availability const* a,
                          AvailabilityAggregator aggr) const {
    switch (aggr) {
      case AvailabilityAggregator_Average: return a->average();
      case AvailabilityAggregator_Median: return a->median();
      case AvailabilityAggregator_Minimum: return a->minimum();
      case AvailabilityAggregator_Quantile90: return a->q90();
      case AvailabilityAggregator_PercentReliable: return a->percent_reliable();
      default: return 0.0;
    }
  }

  Offset<Vector<Offset<BikesharingEdge>>> serialize_edges(
      context& ctx, std::multimap<std::string, bike_edge> const& edges) const {
    std::vector<Offset<BikesharingEdge>> stored;
    for (auto const& pair : edges) {
      auto const& edge = pair.second;
      auto from = serialize_terminal(ctx, edge.from_);
      auto to = serialize_terminal(ctx, edge.to_);

      stored.push_back(CreateBikesharingEdge(
          ctx.b_, from, to, ctx.b_.CreateVectorOfStructs(edge.availability_),
          ctx.b_.CreateString(edge.eva_nr_), edge.walk_duration_,
          edge.bike_duration_));
    }
    return ctx.b_.CreateVector(stored);
  }

  Offset<BikesharingTerminal> serialize_terminal(
      context& ctx, persistable_terminal* terminal) const {
    auto const* t = terminal->get();
    return get_or_create(ctx.terminal_offsets_, t->id()->str(), [&]() {
      motis::Position pos(t->lat(), t->lng());
      return CreateBikesharingTerminal(
          ctx.b_, ctx.b_.CreateString(t->id()->str()),
          ctx.b_.CreateString(t->name()->str()), &pos);
    });
  }

  database const& db_;
  std::vector<std::string> terminal_ids_;
  quadratic_rtree rtree_;
};

bikesharing_search::bikesharing_search(database const& db)
    : impl_(new impl(db)) {}

bikesharing_search::~bikesharing_search() = default;

msg_ptr bikesharing_search::find_connections(
    BikesharingRequest const* req) const {
  return impl_->find_connections(req);
}

}  // namespace bikesharing
}  // namespace motis
