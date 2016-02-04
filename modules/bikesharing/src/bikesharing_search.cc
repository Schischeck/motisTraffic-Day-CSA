#include "motis/bikesharing/bikesharing_search.h"

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

constexpr unsigned long kSecondsPerHour = 3600;

using availability_bucket = BikesharingAvailability;
struct bikesharing_search::impl {

  struct bike_edge {
    int walk_duration;
    int bike_duration;
    std::vector<availability_bucket> availability;

    persistable_terminal* from;
    persistable_terminal* to;
  };

  struct context {
    MessageCreator b;
    std::map<std::string, std::unique_ptr<persistable_terminal>> terminals;
    std::map<std::string, Offset<BikesharingTerminal>> terminal_offsets;
  };

  impl(database const& db) : db_(db) {
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

  msg_ptr find_connections(BikesharingRequest const* req) const {
    context ctx;
    auto const departures = find_departures(ctx, req);
    auto const arrivals = find_arrivals(ctx, req);

    ctx.b.CreateAndFinish(
        MsgContent_BikesharingResponse,
        CreateBikesharingResponse(ctx.b, departures, arrivals).Union());
    return make_msg(ctx.b);
  }

  Offset<Vector<Offset<BikesharingEdge>>> find_departures(
      context& ctx, BikesharingRequest const* req) const {
    auto begin = req->window_begin() - req->window_begin() % kSecondsPerHour;
    auto end = req->window_end();
    auto first_bucket = timestamp_to_bucket(begin);

    std::multimap<std::string, bike_edge> departures;
    foreach_terminal_in_walk_dist(
        req->departure_lng(), req->departure_lat(),
        [&, this](std::string const& id, int walk_dur) {
          auto const& term = load_terminal(ctx, id);
          for (auto const& reachable_t : *term->get()->reachable()) {
            auto other_t = load_terminal(ctx, reachable_t->id()->str());

            for (auto const& station : *other_t->get()->attached()) {
              auto a = get_availability(term->get(), begin, end, first_bucket);
              bike_edge e{walk_dur + station->duration(),
                          reachable_t->duration(), a, term, other_t};
              departures.emplace(id, e);
            }
          }
        });

    return serialize_edges(ctx, departures);
  }

  Offset<Vector<Offset<BikesharingEdge>>> find_arrivals(
      context& ctx, BikesharingRequest const* req) const {
    auto begin = req->window_begin() - req->window_begin() % kSecondsPerHour;
    auto end = req->window_end() + MAX_TRAVEL_TIME_SECONDS;
    auto first_bucket = timestamp_to_bucket(begin);

    std::multimap<std::string, bike_edge> arrivals;
    foreach_terminal_in_walk_dist(
        req->departure_lng(), req->departure_lat(),
        [&, this](std::string const& id, int walk_dur) {
          auto const& term = load_terminal(ctx, id);
          for (auto const& reachable_t : *term->get()->reachable()) {
            auto other_t = load_terminal(ctx, reachable_t->id()->str());

            for (auto const& station : *other_t->get()->attached()) {
              auto a = get_availability(term->get(), begin, end, first_bucket);
              bike_edge e{walk_dur + station->duration(),
                          reachable_t->duration(), a, term, other_t};
              arrivals.emplace(id, e);
            }
          }
        });

    return serialize_edges(ctx, arrivals);
  }

  template <typename F>
  void foreach_terminal_in_walk_dist(double lat, double lng, F func) const {
    spherical_point loc(lng, lat);
    for (auto it =
             rtree_.qbegin(bgi::intersects(generate_box(loc, MAX_WALK_DIST)) &&
                           bgi::satisfies([&loc](value const& v) {
                             return distance_in_m(v.first, loc) < MAX_WALK_DIST;
                           }));
         it != rtree_.qend(); ++it) {
      int walk_dur = distance_in_m(it->first, loc) / WALK_SPEED;  // TODO osrm
      func(terminal_ids_[it->second], walk_dur);
    }
  }

  persistable_terminal* load_terminal(context& c, std::string const& id) const {
    return get_or_create(c.terminals, id, [&]() {
             return make_unique<persistable_terminal>(db_.get(id));
           }).get();
  }

  std::vector<availability_bucket> get_availability(Terminal const* term,
                                                    unsigned long begin,
                                                    unsigned long end,
                                                    size_t bucket) const {
    std::vector<availability_bucket> availability;
    for (auto t = begin; t < end; t += kSecondsPerHour) {
      auto val = term->availability()->Get(bucket++)->minimum();
      availability.push_back({t, t + kSecondsPerHour, val});
    }
    return availability;
  }

  Offset<Vector<Offset<BikesharingEdge>>> serialize_edges(
      context& ctx, std::multimap<std::string, bike_edge> const& edges) const {
    std::vector<Offset<BikesharingEdge>> stored;
    for (auto const& pair : edges) {
      auto const& edge = pair.second;
      auto from = serialize_terminal(ctx, edge.from);
      auto to = serialize_terminal(ctx, edge.to);

      stored.push_back(CreateBikesharingEdge(
          ctx.b, from, to, ctx.b.CreateVectorOfStructs(edge.availability)));
    }
    return ctx.b.CreateVector(stored);
  }

  Offset<BikesharingTerminal> serialize_terminal(
      context& ctx, persistable_terminal* terminal) const {
    auto const* t = terminal->get();
    return get_or_create(ctx.terminal_offsets, t->id()->str(), [&]() {
      return CreateBikesharingTerminal(
          ctx.b, ctx.b.CreateString(t->id()->str()), t->lat(), t->lng(),
          ctx.b.CreateString(t->name()->str()));
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
