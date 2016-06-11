#pragma once

#include "motis/module/message.h"
#include "motis/bikesharing/database.h"
#include "motis/bikesharing/geo_index.h"

namespace motis {
namespace bikesharing {

struct bikesharing_search {
  bikesharing_search(database const&, geo_index const&);
  ~bikesharing_search();

  module::msg_ptr find_connections(BikesharingRequest const*) const;

private:
  struct impl;
  std::unique_ptr<impl> impl_;
};

}  // namespace bikesharing
}  // namespace motis
