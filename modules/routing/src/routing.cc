#include "motis/routing/routing.h"

#include "boost/date_time/gregorian/gregorian_types.hpp"
#include "boost/date_time/posix_time/posix_time.hpp"
#include "boost/program_options.hpp"

#include "motis/core/common/logging.h"
#include "motis/core/common/timing.h"
#include "motis/core/schedule/schedule.h"
#include "motis/core/journey/journeys_to_message.h"
#include "motis/module/context/get_schedule.h"
#include "motis/module/context/motis_call.h"
#include "motis/module/error.h"

#include "motis/routing/additional_edges.h"
#include "motis/routing/error.h"
#include "motis/routing/label/configs.h"
#include "motis/routing/memory_manager.h"
#include "motis/routing/search.h"
#include "motis/routing/start_label_gen.h"

#define LABEL_MEMORY_NUM_BYTES "routing.label_store_size"

namespace p = std::placeholders;
namespace po = boost::program_options;
namespace fbs = flatbuffers;
using namespace motis::logging;
using namespace motis::module;
using namespace motis::guesser;

namespace motis {
namespace routing {

struct memory {
  memory(std::size_t bytes) : in_use_(false), mem_(bytes) {}
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

std::vector<station_node*> get_arrivals(
    fbs::Vector<fbs::Offset<StationPathElement>> const* path) {
  std::vector<station_node*> station_nodes;
  auto const& sched = get_schedule();

  for (auto const& el : *path) {
    std::string station_id;

    if (el->eva_nr()->Length() != 0) {
      station_id = el->eva_nr()->str();
    } else {
      message_creator b;
      b.create_and_finish(
          MsgContent_StationGuesserRequest,
          CreateStationGuesserRequest(b, 1, b.CreateString(el->name()->str()))
              .Union(),
          "/guesser");
      auto msg = motis_call(make_msg(b))->val();
      auto guesses = motis_content(StationGuesserResponse, msg)->guesses();
      if (guesses->size() == 0) {
        throw std::system_error(error::no_guess_for_station);
      }
      station_id = guesses->Get(0)->eva()->str();
    }

    auto const& eva_to_station = sched.eva_to_station_;
    auto it = eva_to_station.find(station_id);
    if (it == end(eva_to_station)) {
      throw std::system_error(error::given_eva_not_available);
    }
    station_nodes.push_back(sched.station_nodes_.at(it->second->index_).get());
  }

  return station_nodes;
}

routing::routing()
    : label_bytes_(static_cast<uint64_t>(8) * 1024 * 1024 * 1024) {}

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

msg_ptr routing::route(msg_ptr const& msg) {
  printf("\nROUTE %s", msg->to_json().c_str());
  auto req = motis_content(RoutingRequest, msg);
  if (req->path()->Length() < 2) {
    throw std::system_error(error::path_length_too_short);
  }

  auto const& sched = get_schedule();
  if (req->interval()->begin() < static_cast<unsigned>(sched.schedule_begin_) ||
      req->interval()->end() >= static_cast<unsigned>(sched.schedule_end_)) {
    throw std::system_error(error::journey_date_not_in_schedule);
  }

  auto path = get_arrivals(req->path());
  auto i_begin =
      unix_to_motistime(sched.schedule_begin_, req->interval()->begin());
  auto i_end = unix_to_motistime(sched.schedule_begin_, req->interval()->end());

  mem_retriever mem(mem_pool_mutex_, mem_pool_, label_bytes_);

  search_query q = {sched,
                    mem.get(),
                    path.at(0),
                    path.at(1),
                    create_additional_edges(req->additional_edges(), sched),
                    i_begin,
                    i_end};

  search_result res;
  switch (req->type()) {
    case Type_LateConnection:
    case Type_OnTrip:
      res = search<ontrip_gen<my_label>, my_label>::get_connections(q);
      break;
    case Type_PreTrip:
      res = search<pretrip_gen<my_label>, my_label>::get_connections(q);
      break;
  }

  return journeys_to_message(res.journeys_, res.stats_.pareto_dijkstra_);
}

}  // namespace routing
}  // namespace motis
