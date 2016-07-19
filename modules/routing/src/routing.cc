#include "motis/routing/routing.h"

#include "boost/date_time/gregorian/gregorian_types.hpp"
#include "boost/date_time/posix_time/posix_time.hpp"
#include "boost/program_options.hpp"

#include "motis/core/common/logging.h"
#include "motis/core/common/timing.h"
#include "motis/core/schedule/schedule.h"
#include "motis/core/access/trip_access.h"
#include "motis/core/access/trip_iterator.h"
#include "motis/core/conv/trip_conv.h"
#include "motis/core/journey/journeys_to_message.h"
#include "motis/module/context/get_schedule.h"
#include "motis/module/context/motis_call.h"
#include "motis/module/error.h"
#include "motis/loader/util.h"

#include "motis/routing/additional_edges.h"
#include "motis/routing/error.h"
#include "motis/routing/label/configs.h"
#include "motis/routing/memory_manager.h"
#include "motis/routing/search.h"
#include "motis/routing/start_label_gen.h"

#define LABEL_MEMORY_NUM_BYTES "routing.label_store_size"

namespace p = std::placeholders;
namespace po = boost::program_options;
using namespace motis::logging;
using namespace motis::module;
using namespace motis::guesser;

namespace motis {
namespace routing {

struct memory {
  explicit memory(std::size_t bytes) : in_use_(false), mem_(bytes) {}
  bool in_use_;
  memory_manager mem_;
};

struct mem_retriever {
  mem_retriever(std::mutex& mutex,
                std::vector<std::unique_ptr<memory>>& mem_pool,
                std::size_t bytes)
      : mutex_(mutex), memory_(retrieve(mem_pool, bytes)) {}

  ~mem_retriever() {
    std::lock_guard<std::mutex> lock(mutex_);
    memory_->in_use_ = false;
  }

  memory_manager& get() { return memory_->mem_; }

private:
  memory* retrieve(std::vector<std::unique_ptr<memory>>& mem_pool,
                   std::size_t bytes) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = std::find_if(begin(mem_pool), end(mem_pool),
                           [](auto&& m) { return !m->in_use_; });
    if (it == end(mem_pool)) {
      mem_pool.emplace_back(new memory(bytes));
      mem_pool.back()->in_use_ = true;
      return mem_pool.back().get();
    }
    it->get()->in_use_ = true;
    return it->get();
  }

  std::mutex& mutex_;
  memory* memory_;
};

routing::routing()
    : label_bytes_(static_cast<std::size_t>(8) * 1024 * 1024 * 1024) {}

routing::~routing() = default;

po::options_description routing::desc() {
  po::options_description desc("Routing Module");
  // clang-format off
  desc.add_options()
    (LABEL_MEMORY_NUM_BYTES,
     po::value<std::size_t>(&label_bytes_)->default_value(label_bytes_),
     "size of the label store in bytes");
  // clang-format on
  return desc;
}

void routing::print(std::ostream& out) const {
  out << "  " << LABEL_MEMORY_NUM_BYTES << ": " << label_bytes_;
}

void routing::init(motis::module::registry& reg) {
  reg.register_op("/routing", std::bind(&routing::route, this, p::_1));
}

station_node const* get_station_node(schedule const& sched,
                                     InputStation const* el) {
  std::string station_id;

  if (el->id()->Length() != 0) {
    station_id = el->id()->str();
  } else {
    message_creator b;
    b.create_and_finish(
        MsgContent_StationGuesserRequest,
        CreateStationGuesserRequest(b, 1, b.CreateString(el->name()->str()))
            .Union(),
        "/guesser");
    auto const msg = motis_call(make_msg(b))->val();
    auto const guesses = motis_content(StationGuesserResponse, msg)->guesses();
    if (guesses->size() == 0) {
      throw std::system_error(error::no_guess_for_station);
    }
    station_id = guesses->Get(0)->id()->str();
  }

  return motis::get_station_node(sched, station_id);
}

node const* get_route_node(schedule const& sched, TripId const* trip,
                           station_node const* station, time arrival_time) {
  auto const stops = access::stops(from_fbs(sched, trip));
  auto const stop_it = std::find_if(
      begin(stops), end(stops), [&](access::trip_stop const& stop) {
        return stop.get_route_node()->station_node_ == station &&
               stop.arr_lcon().a_time_ == arrival_time;
      });
  if (stop_it == end(stops)) {
    throw std::system_error(error::event_not_found);
  }
  return (*stop_it).get_route_node();
}

search_query get_query(schedule const& sched, RoutingRequest const* req) {
  search_query q;

  switch (req->start_type()) {
    case Start_PretripStart: {
      auto start = reinterpret_cast<PretripStart const*>(req->start());
      q.from_ = get_station_node(sched, start->station());
      q.interval_begin_ = unix_to_motistime(sched, start->interval()->begin());
      q.interval_end_ = unix_to_motistime(sched, start->interval()->end());
      break;
    }

    case Start_OntripStationStart: {
      auto start = reinterpret_cast<OntripStationStart const*>(req->start());
      q.from_ = get_station_node(sched, start->station());
      q.interval_begin_ = unix_to_motistime(sched, start->departure_time());
      q.interval_end_ = INVALID_TIME;
      break;
    }

    case Start_OntripTrainStart: {
      auto start = reinterpret_cast<OntripTrainStart const*>(req->start());
      q.from_ = get_route_node(sched, start->trip(),
                               get_station_node(sched, start->station()),
                               unix_to_motistime(sched, start->arrival_time()));
      q.interval_begin_ = unix_to_motistime(sched, start->arrival_time());
      q.interval_end_ = INVALID_TIME;
      break;
    }

    case Start_NONE: assert(false);
  }

  q.sched_ = &sched;
  q.to_ = get_station_node(sched, req->destination());
  q.query_edges_ = create_additional_edges(req->additional_edges(), sched);

  return q;
}

search_result ontrip_search_fwd(search_query const& q, SearchType const t) {
  switch (t) {
    case SearchType_Default:
      return search<search_dir::FWD,
                    ontrip_gen<search_dir::FWD, default_label<search_dir::FWD>>,
                    default_label<search_dir::FWD>>::get_connections(q);
    case SearchType_SingleCriterion:
      return search<
          search_dir::FWD,
          ontrip_gen<search_dir::FWD, single_criterion_label<search_dir::FWD>>,
          single_criterion_label<search_dir::FWD>>::get_connections(q);
    case SearchType_LateConnections:
      return search<
          search_dir::FWD,
          ontrip_gen<search_dir::FWD, late_connections_label<search_dir::FWD>>,
          late_connections_label<search_dir::FWD>>::get_connections(q);
    case SearchType_LateConnectionsTest:
      return search<
          search_dir::FWD,
          ontrip_gen<search_dir::FWD,
                     late_connections_label_for_tests<search_dir::FWD>>,
          late_connections_label_for_tests<search_dir::FWD>>::
          get_connections(q);
    default: break;
  }
  throw std::system_error(error::search_type_not_supported);
}

search_result pretrip_search_fwd(search_query const& q, SearchType const t) {
  switch (t) {
    case SearchType_Default:
      return search<
          search_dir::FWD,
          pretrip_gen<search_dir::FWD, default_label<search_dir::FWD>>,
          default_label<search_dir::FWD>>::get_connections(q);
    case SearchType_SingleCriterion:
      return search<
          search_dir::FWD,
          pretrip_gen<search_dir::FWD, single_criterion_label<search_dir::FWD>>,
          single_criterion_label<search_dir::FWD>>::get_connections(q);
    case SearchType_LateConnections:
      return search<
          search_dir::FWD,
          pretrip_gen<search_dir::FWD, late_connections_label<search_dir::FWD>>,
          late_connections_label<search_dir::FWD>>::get_connections(q);
    case SearchType_LateConnectionsTest:
      return search<
          search_dir::FWD,
          pretrip_gen<search_dir::FWD,
                      late_connections_label_for_tests<search_dir::FWD>>,
          late_connections_label_for_tests<search_dir::FWD>>::
          get_connections(q);
    default: break;
  }
  throw std::system_error(error::search_type_not_supported);
}

search_result ontrip_search_bwd(search_query const& q, SearchType const t) {
  switch (t) {
    case SearchType_Default:
      return search<search_dir::BWD,
                    ontrip_gen<search_dir::BWD, default_label<search_dir::BWD>>,
                    default_label<search_dir::BWD>>::get_connections(q);
    case SearchType_SingleCriterion:
      return search<
          search_dir::BWD,
          ontrip_gen<search_dir::BWD, single_criterion_label<search_dir::BWD>>,
          single_criterion_label<search_dir::BWD>>::get_connections(q);
    case SearchType_LateConnections:
      return search<
          search_dir::BWD,
          ontrip_gen<search_dir::BWD, late_connections_label<search_dir::BWD>>,
          late_connections_label<search_dir::BWD>>::get_connections(q);
    case SearchType_LateConnectionsTest:
      return search<
          search_dir::BWD,
          ontrip_gen<search_dir::BWD,
                     late_connections_label_for_tests<search_dir::BWD>>,
          late_connections_label_for_tests<search_dir::BWD>>::
          get_connections(q);
    default: break;
  }
  throw std::system_error(error::search_type_not_supported);
}

search_result pretrip_search_bwd(search_query const& q, SearchType const t) {
  switch (t) {
    case SearchType_Default:
      return search<
          search_dir::BWD,
          pretrip_gen<search_dir::BWD, default_label<search_dir::BWD>>,
          default_label<search_dir::BWD>>::get_connections(q);
    case SearchType_SingleCriterion:
      return search<
          search_dir::BWD,
          pretrip_gen<search_dir::BWD, single_criterion_label<search_dir::BWD>>,
          single_criterion_label<search_dir::BWD>>::get_connections(q);
    case SearchType_LateConnections:
      return search<
          search_dir::BWD,
          pretrip_gen<search_dir::BWD, late_connections_label<search_dir::BWD>>,
          late_connections_label<search_dir::BWD>>::get_connections(q);
    case SearchType_LateConnectionsTest:
      return search<
          search_dir::BWD,
          pretrip_gen<search_dir::BWD,
                      late_connections_label_for_tests<search_dir::BWD>>,
          late_connections_label_for_tests<search_dir::BWD>>::
          get_connections(q);
    default: break;
  }
  throw std::system_error(error::search_type_not_supported);
}

search_result find_connections(search_query const& q, Start s,
                               SearchType const t, SearchDir d) {
  switch (s) {
    case Start_PretripStart:
      if (d == SearchDir_Forward) {
        return pretrip_search_fwd(q, t);
      } else {
        return pretrip_search_bwd(q, t);
      }
    case Start_OntripStationStart:
    case Start_OntripTrainStart:
      if (d == SearchDir_Forward) {
        return ontrip_search_fwd(q, t);
      } else {
        return ontrip_search_bwd(q, t);
      }
      break;
    case Start_NONE: assert(false);
  }
  return {};
}

msg_ptr routing::route(msg_ptr const& msg) {
  auto const req = motis_content(RoutingRequest, msg);
  auto const& sched = get_schedule();
  auto query = get_query(sched, req);

  mem_retriever mem(mem_pool_mutex_, mem_pool_, label_bytes_);
  query.mem_ = &mem.get();

  auto const res = find_connections(query, req->start_type(),
                                    req->search_type(), req->search_dir());

  message_creator fbb;
  fbb.create_and_finish(
      MsgContent_RoutingResponse,
      CreateRoutingResponse(
          fbb, res.stats_.pareto_dijkstra_,
          fbb.CreateVector(loader::transform_to_vec(
              res.journeys_,
              [&](journey const& j) { return to_connection(fbb, j); })))
          .Union());
  return make_msg(fbb);
}

}  // namespace routing
}  // namespace motis
