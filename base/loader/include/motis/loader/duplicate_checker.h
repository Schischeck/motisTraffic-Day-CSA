#pragma once

#include <vector>

#include "motis/core/schedule/schedule.h"

namespace motis {
namespace loader {

struct duplicate_checker {
  explicit duplicate_checker(schedule&);

  void remove_duplicates();
  void remove_duplicates(station_node*);
  unsigned get_duplicate_count();

private:
  void handle_duplicates(std::vector<std::pair<time, light_connection*>>&);
  bool is_duplicate_event(std::pair<time, light_connection*> const&,
                          std::pair<time, light_connection*> const&) const;
  void set_new_service_num(light_connection*);

  schedule& schedule_;
  uint32_t last_service_num_;
  unsigned duplicate_count_;
};

}  // namespace loader
}  // namespace motis
