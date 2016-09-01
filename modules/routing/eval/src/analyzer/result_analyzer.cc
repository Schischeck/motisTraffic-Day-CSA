#include <fstream>
#include <iostream>
#include <vector>

#include "motis/module/message.h"
#include "motis/eval/analyzer/quantile.h"
#include "motis/protocol/RoutingResponse_generated.h"
#include "motis/routing/allocator.h"

using namespace motis;
using namespace motis::module;
using namespace motis::routing;
using namespace motis::eval;

struct response {
  explicit response(RoutingResponse const* r)
      : labels_until_first_(
            r->statistics()->labels_popped_until_first_result()),
        labels_after_last_(r->statistics()->labels_popped_after_last_result()),
        labels_created_(r->statistics()->labels_created()),
        pd_time_(r->statistics()->pareto_dijkstra()),
        start_labels_(r->statistics()->start_label_count()),
        con_count_(r->connections()->size()),
        max_label_quit_(r->statistics()->max_label_quit()),
        total_time_(r->statistics()->total_calculation_time()),
        travel_time_lb_time_(r->statistics()->travel_time_lb()),
        transfers_lb_time_(r->statistics()->transfers_lb()),
        allocations_(get_stats(r->statistics(), "freelist").get_allocs()),
        deallocations_(get_stats(r->statistics(), "freelist").get_deallocs()),
        freelist_hits_(get_stats(r->statistics(), "freelist").get_hits()),
        block_allocations_(
            get_stats(r->statistics(), "inc_block").get_allocs()) {}

  static mem_stats get_stats(Statistics const* stats, char const* name) {
    auto it = std::find_if(std::begin(*stats->mem_stats()),
                           std::end(*stats->mem_stats()),
                           [&name](MemStats const* ms) {
                             return std::strcmp(ms->name()->c_str(), name) == 0;
                           });
    if (it == std::end(*stats->mem_stats())) {
      return mem_stats();
    } else {
      mem_stats s;
      s.count_allocation(it->allocations());
      s.count_deallocation(it->deallocations());
      s.count_hit(it->hits());
      return s;
    }
  }

  unsigned labels_until_first_;
  unsigned labels_after_last_;
  unsigned labels_created_;
  unsigned pd_time_;
  unsigned start_labels_;
  unsigned con_count_;
  bool max_label_quit_;
  unsigned total_time_;
  unsigned travel_time_lb_time_;
  unsigned transfers_lb_time_;

  unsigned allocations_;
  unsigned deallocations_;
  unsigned freelist_hits_;
  unsigned block_allocations_;
};

int main(int argc, char* argv[]) {
  if (argc != 2) {
    std::cout << "usage: " << argv[0] << " {results.txt file}\n";
    return 0;
  }

  std::vector<response> responses;

  std::ifstream in(argv[1]);
  std::string line;

  uint64_t max_label_quit = 0;
  uint64_t pd_time_sum = 0;
  uint64_t total_time_sum = 0;
  uint64_t travel_time_lb_time_sum = 0;
  uint64_t transfers_lb_time_sum = 0;
  uint64_t count = 0;
  uint64_t no_con_count = 0;
  uint64_t labels_popped_until_first_result = 0;
  uint64_t labels_popped_after_last_result = 0;
  uint64_t no_labels_created = 0;
  uint64_t start_labels = 0;
  uint64_t allocations = 0;
  uint64_t deallocations = 0;
  uint64_t freelist_hits = 0;
  uint64_t block_allocations = 0;

  while (in.peek() != EOF && !in.eof()) {
    std::getline(in, line);

    auto res_msg = make_msg(line);
    response res(motis_content(RoutingResponse, res_msg));

    if (res.max_label_quit_) {
      ++max_label_quit;
    }

    if (res.con_count_ == 0) {
      ++no_con_count;
      continue;
    }

    responses.push_back(res);
    labels_popped_until_first_result += res.labels_until_first_;
    labels_popped_after_last_result += res.labels_after_last_;
    no_labels_created += res.labels_created_;
    pd_time_sum += res.pd_time_;
    total_time_sum += res.total_time_;
    travel_time_lb_time_sum += res.travel_time_lb_time_;
    transfers_lb_time_sum += res.transfers_lb_time_;
    start_labels += res.start_labels_;
    allocations += res.allocations_;
    deallocations += res.deallocations_;
    freelist_hits += res.freelist_hits_;
    block_allocations += res.block_allocations_;
    ++count;
  }

  if (responses.empty()) {
    std::cout << "no responses found\n";
    std::cout << "responses with no connection = " << no_con_count << "\n";
    return 0;
  }

  std::cout << "   total count: " << count << "\n"
            << "       no conn: " << no_con_count << "\n"
            << "max label quit: " << max_label_quit << "\n"
            << "\n\n";

  std::cout << "    average core routing time [ms]: "
            << pd_time_sum / static_cast<double>(count) << "\n"
            << "90 quantile core routing time [ms]: "
            << quantile(&response::pd_time_, responses, 0.9f) << "\n"
            << "80 quantile core routing time [ms]: "
            << quantile(&response::pd_time_, responses, 0.8f) << "\n"
            << "50 quantile core routing time [ms]: "
            << quantile(&response::pd_time_, responses, 0.5f) << "\n\n";

  std::cout << "    average total time [ms]: "
            << total_time_sum / static_cast<double>(count) << "\n"
            << "90 quantile total time [ms]: "
            << quantile(&response::total_time_, responses, 0.9f) << "\n"
            << "80 quantile total time [ms]: "
            << quantile(&response::total_time_, responses, 0.8f) << "\n"
            << "50 quantile total time [ms]: "
            << quantile(&response::total_time_, responses, 0.5f) << "\n\n";

  std::cout << "    average travel time lb time [ms]: "
            << travel_time_lb_time_sum / static_cast<double>(count) << "\n"
            << "90 quantile travel time lb time [ms]: "
            << quantile(&response::travel_time_lb_time_, responses, 0.9f)
            << "\n"
            << "80 quantile travel time lb time [ms]: "
            << quantile(&response::travel_time_lb_time_, responses, 0.8f)
            << "\n"
            << "50 quantile travel time lb time [ms]: "
            << quantile(&response::travel_time_lb_time_, responses, 0.5f)
            << "\n\n";

  std::cout << "    average transfers lb time [ms]: "
            << transfers_lb_time_sum / static_cast<double>(count) << "\n"
            << "90 quantile transfers lb time [ms]: "
            << quantile(&response::transfers_lb_time_, responses, 0.9f) << "\n"
            << "80 quantile transfers lb time [ms]: "
            << quantile(&response::transfers_lb_time_, responses, 0.8f) << "\n"
            << "50 quantile transfers lb time [ms]: "
            << quantile(&response::transfers_lb_time_, responses, 0.5f)
            << "\n\n";

  std::cout << "    average number of start labels: "
            << start_labels / static_cast<double>(count) << "\n"
            << "90 quantile number of start labels: "
            << quantile(&response::start_labels_, responses, 0.9f) << "\n"
            << "80 quantile number of start labels: "
            << quantile(&response::start_labels_, responses, 0.8f) << "\n"
            << "50 quantile number of start labels: "
            << quantile(&response::start_labels_, responses, 0.5f) << "\n\n";

  std::cout << "    average number of labels created: "
            << no_labels_created / static_cast<double>(count) << "\n"
            << "90 quantile number of labels created: "
            << quantile(&response::labels_created_, responses, 0.9f) << "\n"
            << "80 quantile number of labels created: "
            << quantile(&response::labels_created_, responses, 0.8f) << "\n"
            << "50 quantile number of labels created: "
            << quantile(&response::labels_created_, responses, 0.5f) << "\n\n";

  std::cout << "    average number of allocations: "
            << allocations / static_cast<double>(count) << "\n"
            << "90 quantile number of allocations: "
            << quantile(&response::allocations_, responses, 0.9f) << "\n"
            << "80 quantile number of allocations: "
            << quantile(&response::allocations_, responses, 0.8f) << "\n"
            << "50 quantile number of allocations: "
            << quantile(&response::allocations_, responses, 0.5f) << "\n\n";

  std::cout << "    average number of deallocations: "
            << deallocations / static_cast<double>(count) << "\n"
            << "90 quantile number of deallocations: "
            << quantile(&response::deallocations_, responses, 0.9f) << "\n"
            << "80 quantile number of deallocations: "
            << quantile(&response::deallocations_, responses, 0.8f) << "\n"
            << "50 quantile number of deallocations: "
            << quantile(&response::deallocations_, responses, 0.5f) << "\n\n";

  std::cout << "    average number of freelist hits: "
            << freelist_hits / static_cast<double>(count) << "\n"
            << "90 quantile number of freelist hits: "
            << quantile(&response::freelist_hits_, responses, 0.9f) << "\n"
            << "80 quantile number of freelist hits: "
            << quantile(&response::freelist_hits_, responses, 0.8f) << "\n"
            << "50 quantile number of freelist hits: "
            << quantile(&response::freelist_hits_, responses, 0.5f) << "\n\n";

  std::cout << "    average number of block allocations: "
            << block_allocations / static_cast<double>(count) << "\n"
            << "90 quantile number of block allocations: "
            << quantile(&response::block_allocations_, responses, 0.9f) << "\n"
            << "80 quantile number of block allocations: "
            << quantile(&response::block_allocations_, responses, 0.8f) << "\n"
            << "50 quantile number of block allocations: "
            << quantile(&response::block_allocations_, responses, 0.5f)
            << "\n\n";

  std::cout << "avg labels popped:\n";
  std::cout << "\tuntil first result: "
            << labels_popped_until_first_result / static_cast<double>(count)
            << " ("
            << labels_popped_until_first_result /
                   static_cast<double>(no_labels_created)
            << ")\n";
  std::cout << "\t after last result: "
            << labels_popped_after_last_result / static_cast<double>(count)
            << " ("
            << labels_popped_after_last_result /
                   static_cast<double>(no_labels_created)
            << ")\n";
}
