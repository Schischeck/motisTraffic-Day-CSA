#include "motis/routing/routing.h"

#include "boost/date_time/gregorian/gregorian_types.hpp"
#include "boost/date_time/posix_time/posix_time.hpp"
#include "boost/program_options.hpp"

#include "motis/core/common/logging.h"
#include "motis/core/common/timing.h"
#include "motis/core/common/timing.h"
#include "motis/core/common/transform_to_vec.h"
#include "motis/core/schedule/schedule.h"
#include "motis/core/journey/journeys_to_message.h"
#include "motis/module/context/get_schedule.h"

#include "motis/routing/additional_edges.h"
#include "motis/routing/build_query.h"
#include "motis/routing/error.h"
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

msg_ptr routing::route(msg_ptr const& msg) {
  MOTIS_START_TIMING(routing_timing);

  auto const req = motis_content(RoutingRequest, msg);
  auto const& sched = get_schedule();
  auto query = build_query(sched, req);

  mem_retriever mem(mem_pool_mutex_, mem_pool_, label_bytes_);
  query.mem_ = &mem.get();

  auto res = search_dispatch(query, req->start_type(), req->search_type(),
                             req->search_dir());

  MOTIS_STOP_TIMING(routing_timing);
  res.stats_.total_calculation_time_ = MOTIS_TIMING_MS(routing_timing);

  message_creator fbb;
  auto const stats = to_fbs(res.stats_);
  fbb.create_and_finish(
      MsgContent_RoutingResponse,
      CreateRoutingResponse(fbb, &stats, fbb.CreateVector(transform_to_vec(
                                             res.journeys_,
                                             [&](journey const& j) {
                                               return to_connection(fbb, j);
                                             })))
          .Union());
  return make_msg(fbb);
}

}  // namespace routing
}  // namespace motis
