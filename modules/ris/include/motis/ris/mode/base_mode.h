#pragma once

#include <ctime>
#include <set>
#include <string>

#include "motis/core/schedule/schedule.h"
#include "motis/module/context/get_schedule.h"
#include "motis/module/context/motis_publish.h"
#include "motis/module/message.h"

#include "motis/ris/database.h"

namespace motis {
namespace module {

struct registry;

}  // namespace module

namespace ris {
struct config;
struct ris_message;

namespace mode {

struct base_mode {  // hint: strategy pattern ;)
  explicit base_mode(config* conf) : conf_(conf) {}
  virtual ~base_mode() = default;

  base_mode(base_mode const&) = delete;
  base_mode& operator=(base_mode const&) = delete;

  base_mode(base_mode&&) = delete;
  base_mode& operator=(base_mode&&) = delete;

  virtual void init(motis::module::registry&);
  virtual void init_async();

protected:
  void forward(std::time_t);

  template <typename Fn>
  void system_time_forward(std::time_t const new_system_time, Fn fn) {
    std::time_t last_update_timestamp = fn();

    auto& sched = motis::module::get_schedule();
    sched.system_time_ = new_system_time;
    sched.last_update_timestamp_ =
        std::max(sched.last_update_timestamp_, last_update_timestamp);

    motis_publish(motis::module::make_no_msg("/ris/system_time_changed"));
  }

  db_ptr db_;
  config* conf_;

  std::set<std::string> read_files_;
};

}  // namespace mode
}  // namespace ris
}  // namespace motis
