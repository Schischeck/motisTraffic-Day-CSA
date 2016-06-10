#pragma once

#include <memory>
#include <string>
#include <vector>

#include "motis/bikesharing/database.h"

namespace motis {
namespace bikesharing {

struct close_terminal {
  close_terminal(std::string id, double distance)
      : id_(std::move(id)), distance_(distance){};

  std::string id_;
  double distance_;
};

struct geo_index {
  explicit geo_index(database const&);
  ~geo_index();

  std::vector<close_terminal> get_terminals(double const lat, double const lng,
                                            double const radius) const;

private:
  struct impl;
  std::unique_ptr<impl> impl_;
};

}  // namespace bikesharing
}  // namespace motis
