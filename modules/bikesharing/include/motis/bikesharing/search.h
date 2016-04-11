#pragma once

#include "motis/module/message.h"
#include "motis/bikesharing/database.h"

namespace motis {
namespace bikesharing {

struct bikesharing_search {

  bikesharing_search(database const& db);
  ~bikesharing_search();

  module::msg_ptr find_connections(BikesharingRequest const* req) const;

private:
  struct impl;
  std::unique_ptr<impl> impl_;
};

}  // namespace bikesharing
}  // namespace bikesharing
