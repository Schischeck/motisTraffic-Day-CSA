#pragma once

#include "motis/module/module.h"
#include "motis/ris/mode/base_mode.h"

namespace motis {
namespace ris {
namespace mode {

struct simulation_mode : public base_mode {

  simulation_mode(ris* module) : base_mode(module), simulation_time_(0) {}

  virtual void init_async() override;
  virtual void on_msg(motis::module::msg_ptr, motis::module::sid,
                      motis::module::callback) override;

private:
  void forward_time(std::time_t const, std::time_t const, std::time_t const,
                    std::time_t const, motis::module::callback);

  std::time_t simulation_time_;
};

}  // namespace mode
}  // namespace ris
}  // namespace motis
