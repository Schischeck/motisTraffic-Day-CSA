#pragma once

#include <memory>
#include <mutex>
#include <vector>

#include "motis/module/module.h"

namespace motis {
namespace routing {

struct memory;

struct routing : public motis::module::module {
  routing();
  ~routing() override;
  std::string name() const override { return "routing"; }

  boost::program_options::options_description desc() override;
  void print(std::ostream& out) const override;
  bool empty_config() const override { return false; }

  void init(motis::module::registry&) override;

private:
  motis::module::msg_ptr ontrip_train(motis::module::msg_ptr const&);
  motis::module::msg_ptr route(motis::module::msg_ptr const&);
  motis::module::msg_ptr trip_to_connection(motis::module::msg_ptr const&);

  std::size_t max_label_bytes_;
  std::mutex mem_pool_mutex_;
  std::vector<std::unique_ptr<memory>> mem_pool_;
};

}  // namespace routing
}  // namespace motis
