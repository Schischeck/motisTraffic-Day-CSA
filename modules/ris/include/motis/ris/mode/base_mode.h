#pragma once

#include <set>
#include <string>

#include "motis/ris/database.h"

namespace motis {
namespace module {

struct registry;

} // namespace module

namespace ris {
struct config;

namespace mode {

struct base_mode {  // hint: strategy pattern ;)
  base_mode(config* conf) : conf_(conf) {}
  virtual ~base_mode() = default;

  virtual void init(motis::module::registry&);
  virtual void init_async();

protected:
  db_ptr db_;
  config* conf_;
  std::set<std::string> read_files_;
};

}  // namespace mode
}  // namespace ris
}  // namespace motis
