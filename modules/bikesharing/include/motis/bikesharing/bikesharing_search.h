#pragma once

#include "motis/bikesharing/database.h"
#include "motis/module/message.h"

namespace motis {
namespace bikesharing {

struct bikesharing_search {

  bikesharing_search(database const& db);
  ~bikesharing_search();

  module::msg_ptr find_connections(BikesharingRequest const* req) const;

private:
  struct bikesharing_search_impl;
  std::unique_ptr<bikesharing_search_impl> impl_;
};

}  // namespace bikesharing
}  // namespace bikesharing
