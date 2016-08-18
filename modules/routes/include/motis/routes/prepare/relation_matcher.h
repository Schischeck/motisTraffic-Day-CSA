#pragma once

#include <map>
#include <string>
#include <vector>

#include "motis/routes/prepare/osm_util.h"
#include "motis/schedule-format/Schedule_generated.h"

namespace motis {
namespace routes {

class relation_matcher {

public:
  relation_matcher(motis::loader::Schedule const *sched,
                   std::string const &osm_file);

  void find_perfect_matches();

  void find_segment_matches();

private:
  void load_osm();
  // bool nodes_in_order(segment_match sm);

  std::vector<relation> relations_;

  std::map<int64_t, std::unique_ptr<way>> ways_;
  std::map<int64_t, std::vector<node*>> pending_nodes_;

  motis::loader::Schedule const *sched_;
  std::string const &osm_file_;
};

} // namespace routes
} // namespace motis
