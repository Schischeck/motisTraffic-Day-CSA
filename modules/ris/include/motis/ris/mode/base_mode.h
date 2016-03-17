#pragma once

#include <set>

#include "motis/module/module.h"

namespace motis {
namespace ris {
struct ris;

namespace mode {

struct base_mode {  // hint: strategy pattern ;)

  base_mode(ris* module) : module_(module) {}

  virtual void init_async();
  virtual void on_msg(motis::module::msg_ptr, motis::module::sid,
                      motis::module::callback) = 0;

protected:
  ris* module_;
  std::set<std::string> read_files_;
};

}  // namespace mode
}  // namespace ris
}  // namespace motis
