#pragma once

#include <vector>

#include "motis/core/schedule/schedule.h"

namespace motis {
namespace loader {

struct duplicate_checker {
  duplicate_checker(schedule&, bool ignore_local_traffic = true);

  void remove_duplicates();
  void remove_duplicates(station_node*);
  unsigned get_duplicate_count();

private:
  void handle_duplicates(std::vector<std::pair<time, light_connection*>>&);
  bool is_duplicate_event(std::pair<time, light_connection*> const&,
                          std::pair<time, light_connection*> const&) const;
  void set_new_service_num(light_connection*);

  schedule& schedule_;
  int last_service_num_;
  bool ignore_local_traffic_;
  unsigned duplicate_count_;
};

}  // namespace loader
}  // namespace motis
