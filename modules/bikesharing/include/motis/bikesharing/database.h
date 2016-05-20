#pragma once

#include <memory>
#include <vector>

#include "motis/bikesharing/dbschema/Summary_generated.h"
#include "motis/bikesharing/dbschema/Terminal_generated.h"

#include "motis/core/common/typed_flatbuffer.h"
#include "motis/bikesharing/terminal.h"

namespace motis {
namespace bikesharing {

using persistable_terminal = typed_flatbuffer<Terminal>;
using bikesharing_summary = typed_flatbuffer<Summary>;

persistable_terminal convert_terminal(
    terminal const& terminal, hourly_availabilities const& availabilities,
    std::vector<close_location> const& attached,
    std::vector<close_location> const& reachable);

bikesharing_summary make_summary(std::vector<terminal> const& terminals);

struct database {
  explicit database(std::string const& path);
  ~database();

  persistable_terminal get(std::string const& id) const;
  void put(std::vector<persistable_terminal> const& terminals);

  bikesharing_summary get_summary() const;
  void put_summary(bikesharing_summary const& summary);

  struct database_impl;
  std::unique_ptr<database_impl> impl_;
};

}  // namespace bikesharing
}  // namespace motis
