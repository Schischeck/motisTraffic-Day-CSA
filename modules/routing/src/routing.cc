#include "motis/routing/routing.h"

#include "boost/date_time/gregorian/gregorian_types.hpp"
#include "boost/date_time/posix_time/posix_time.hpp"
#include "boost/program_options.hpp"

#include "motis/core/common/logging.h"
#include "motis/core/common/timing.h"
#include "motis/core/common/timing.h"
#include "motis/core/common/transform_to_vec.h"
#include "motis/core/schedule/schedule.h"
#include "motis/core/access/edge_access.h"
#include "motis/core/conv/trip_conv.h"
#include "motis/core/journey/journeys_to_message.h"
#include "motis/module/context/get_schedule.h"

#include "motis/routing/additional_edges.h"
#include "motis/routing/build_query.h"
#include "motis/routing/error.h"
#include "motis/routing/label/configs.h"
#include "motis/routing/label/configs.h"
#include "motis/routing/mem_manager.h"
#include "motis/routing/mem_retriever.h"
#include "motis/routing/search.h"
#include "motis/routing/search_dispatch.h"
#include "motis/routing/start_label_gen.h"

#define LABEL_MEMORY_NUM_BYTES "routing.label_store_size"

namespace p = std::placeholders;
namespace po = boost::program_options;
using namespace motis::logging;
using namespace motis::module;

namespace motis {
namespace routing {

routing::routing() : max_label_bytes_(32 * 1024 * 1024) {}

routing::~routing() = default;

po::options_description routing::desc() {
  po::options_description desc("Routing Module");
  // clang-format off
  desc.add_options()
    (LABEL_MEMORY_NUM_BYTES,
     po::value<std::size_t>(&max_label_bytes_)->default_value(max_label_bytes_),
     "max size of the label store in bytes");
  // clang-format on
  return desc;
}

void routing::print(std::ostream& out) const {
  out << "  " << LABEL_MEMORY_NUM_BYTES << ": " << max_label_bytes_;
}

void routing::init(motis::module::registry& reg) {
  reg.register_op("/routing", std::bind(&routing::route, this, p::_1));
  reg.register_op("/trip_to_connection",
                  std::bind(&routing::trip_to_connection, this, p::_1));
}

msg_ptr routing::route(msg_ptr const& msg) {
  MOTIS_START_TIMING(routing_timing);

  auto const req = motis_content(RoutingRequest, msg);
  auto const& sched = get_schedule();
  auto query = build_query(sched, req);

  mem_retriever mem(mem_pool_mutex_, mem_pool_, max_label_bytes_);
  query.mem_ = &mem.get();

  auto res = search_dispatch(query, req->start_type(), req->search_type(),
                             req->search_dir());

  MOTIS_STOP_TIMING(routing_timing);
  res.stats_.total_calculation_time_ = MOTIS_TIMING_MS(routing_timing);
  res.stats_.num_bytes_in_use_ = query.mem_->get_num_bytes_in_use();

  message_creator fbb;
  fbb.create_and_finish(
      MsgContent_RoutingResponse,
      CreateRoutingResponse(
          fbb, to_fbs(fbb, res.stats_),
          fbb.CreateVector(transform_to_vec(
              res.journeys_,
              [&](journey const& j) { return to_connection(fbb, j); })),
          motis_to_unixtime(sched, res.interval_begin_),
          motis_to_unixtime(sched, res.interval_end_))
          .Union());
  return make_msg(fbb);
}

msg_ptr routing::trip_to_connection(msg_ptr const& msg) {
  using label = default_label<search_dir::FWD>;

  auto const& sched = get_schedule();
  auto trp = from_fbs(sched, motis_content(TripId, msg));

  auto const first = trp->edges_->front()->from_;
  auto const last = trp->edges_->back()->to_;

  auto const e_0 = make_foot_edge(nullptr, first->get_station());
  auto const e_1 = make_foot_edge(first->get_station(), first);
  auto const e_n = make_foot_edge(last, last->get_station());

  auto const dep_time = get_lcon(trp->edges_->front(), trp->lcon_idx_).d_time_;

  auto const make_label = [&](label* pred, edge const* e,
                              light_connection const* lcon, time now) {
    auto l = label();
    l.pred_ = pred;
    l.edge_ = e;
    l.connection_ = lcon;
    l.start_ = dep_time;
    l.now_ = now;
    l.dominated_ = false;
    return l;
  };

  auto labels = std::vector<label>{trp->edges_->size() + 3};
  labels[0] = make_label(nullptr, &e_0, nullptr, dep_time);
  labels[1] = make_label(&labels[0], &e_1, nullptr, dep_time);

  int i = 2;
  for (auto const& e : *trp->edges_) {
    auto const& lcon = get_lcon(e, trp->lcon_idx_);
    labels[i] = make_label(&labels[i - 1], e, &lcon, lcon.a_time_);
    ++i;
  }

  labels[i] = make_label(&labels[i - 1], &e_n, nullptr, labels[i - 1].now_);

  message_creator fbb;
  fbb.create_and_finish(
      MsgContent_Connection,
      to_connection(
          fbb, output::labels_to_journey(sched, &labels[i], search_dir::FWD))
          .Union());
  return make_msg(fbb);
}

}  // namespace routing
}  // namespace motis
