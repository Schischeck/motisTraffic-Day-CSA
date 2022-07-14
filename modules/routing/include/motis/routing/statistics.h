#pragma once

#include "motis/protocol/RoutingResponse_generated.h"

#include <ostream>

namespace motis {
namespace routing {

struct statistics {
  statistics()
      : labels_created_(0),
        labels_popped_(0),
        labels_dominated_by_results_(0),
        labels_filtered_(0),
        labels_dominated_by_former_labels_(0),
        labels_dominated_by_later_labels_(0),
        labels_popped_until_first_result_(-1),
        labels_popped_after_last_result_(0),
        priority_queue_max_size_(0),
        start_label_count_(0),
        labels_equals_popped_(0),
        max_label_quit_(false),
        travel_time_lb_(0),
        transfers_lb_(0),
        price_l_b_(0),
        total_calculation_time_(0),
        pareto_dijkstra_(0),
        num_bytes_in_use_(0) {}

  explicit statistics(int travel_time_lb) : statistics() {
    travel_time_lb_ = travel_time_lb;
  }

  std::size_t labels_created_;
  int labels_popped_, labels_dominated_by_results_;
  int labels_filtered_, labels_dominated_by_former_labels_,
      labels_dominated_by_later_labels_;
  int labels_popped_until_first_result_, labels_popped_after_last_result_;
  int priority_queue_max_size_;
  int start_label_count_;
  int labels_equals_popped_;
  bool max_label_quit_;
  int travel_time_lb_;
  int transfers_lb_;
  int price_l_b_;
  int total_calculation_time_;
  int pareto_dijkstra_;
  int num_bytes_in_use_;

  // friend Statistics to_fbs(statistics const& s) {
  // return Statistics(
  // s.max_label_quit_, s.labels_created_, s.start_label_count_,
  // s.labels_popped_, s.labels_equals_popped_, s.labels_filtered_,
  // s.labels_dominated_by_results_, s.labels_dominated_by_former_labels_,
  // s.labels_dominated_by_later_labels_,
  // s.labels_popped_until_first_result_, s.labels_popped_after_last_result_,
  // s.priority_queue_max_size_, s.travel_time_lb_, s.transfers_lb_,
  // s.total_calculation_time_, s.pareto_dijkstra_, s.num_bytes_in_use_);
  // }
  friend flatbuffers::Offset<Statistics> to_fbs(
      flatbuffers::FlatBufferBuilder& fbb, char const* category,
      statistics const& s) {
    std::vector<flatbuffers::Offset<StatisticsEntry>> stats{};

    auto const add_entry = [&](char const* key, auto const val) {
      if (val != 0U) {
        stats.emplace_back(
            CreateStatisticsEntry(fbb, fbb.CreateString(key), val));
      }
    };

    add_entry("labels_created", s.labels_created_);
    add_entry("labels_dominated_by_former_labels",
              s.labels_dominated_by_former_labels_);
    add_entry("labels_dominated_by_later_labels",
              s.labels_dominated_by_later_labels_);
    add_entry("labels_dominated_by_results", s.labels_dominated_by_results_);
    add_entry("labels_equals_popped", s.labels_equals_popped_);
    add_entry("labels_filtered", s.labels_filtered_);
    add_entry("labels_popped_after_last_result",
              s.labels_popped_after_last_result_);
    add_entry("labels_popped", s.labels_popped_);
    add_entry("labels_popped_until_first_result",
              s.labels_popped_until_first_result_);
    // add_entry("labels_to_journey", s.labels_to_journey_);
    add_entry("max_label_quit", s.max_label_quit_ ? 1 : 0);
    add_entry("num_bytes_in_use", s.num_bytes_in_use_);
    add_entry("pareto_dijkstra", s.pareto_dijkstra_);
    add_entry("priority_queue_max_size", s.priority_queue_max_size_);
    add_entry("start_label_count", s.start_label_count_);
    add_entry("total_calculation_time", s.total_calculation_time_);
    add_entry("transfers_lb", s.transfers_lb_);
    add_entry("travel_time_lb", s.travel_time_lb_);
    add_entry("price_l_b_", s.price_l_b_);
    // add_entry("interval_extensions", s.interval_extensions_);

    return CreateStatistics(fbb, fbb.CreateString(category),
                            fbb.CreateVectorOfSortedTables(&stats));
  }
};

}  // namespace routing
}  // namespace motis
