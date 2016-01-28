#pragma once

#include <memory>
#include <vector>

#include "motis/bikesharing/dbschema/Terminal_generated.h"

#include "motis/core/common/typed_flatbuffer.h"
#include "motis/bikesharing/terminal.h"

namespace motis {
namespace bikesharing {

using persistable_terminal = typed_flatbuffer<Terminal>;

persistable_terminal convert_terminal(
    terminal const& terminal, hourly_availabilities const& availabilities,
    std::vector<close_location> const& attached,
    std::vector<close_location> const& reachable);

class database {
public:
  database(std::string const& path);
  ~database();

  persistable_terminal get(int id);
  void put(std::vector<persistable_terminal> const& terminals);

private:
  struct database_impl;
  std::unique_ptr<database_impl> impl_;
};

}  // namespace bikesharing
}  // namespace motis
