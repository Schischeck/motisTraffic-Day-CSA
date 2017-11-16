#pragma once

#include <ctime>
#include <string>

#include "conf/holder.h"

#include "motis/module/module.h"

namespace motis {
namespace ris {
namespace mode {

struct base_mode;

}  // namespace mode

enum class mode_t { LIVE, SIMULATION, TEST };

struct config {
  mode_t mode_{mode_t::LIVE};

  std::string input_folder_;
  std::string database_file_;

  // live mode
  int update_interval_{0};
  int max_days_{0};

  // simulation mode
  conf::holder<std::time_t> sim_init_time_{0};
};

struct ris final : public motis::module::module {
  ris();
  ~ris() override;

  ris(ris const&) = delete;
  ris& operator=(ris const&) = delete;

  ris(ris&&) = delete;
  ris& operator=(ris&&) = delete;

  std::string name() const override { return "ris"; }
  void init(motis::module::registry&) override;

private:
  config conf_;
  std::unique_ptr<mode::base_mode> active_mode_;
};

}  // namespace ris
}  // namespace motis
