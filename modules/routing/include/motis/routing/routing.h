#pragma once

#include "motis/module/module.h"

#include "motis/routing/memory_manager.h"
#include "motis/routing/label.h"
#include "motis/routing/arrival.h"

namespace motis {
namespace routing {

struct routing : public motis::module::module {
  routing();
  virtual ~routing() {}

  virtual boost::program_options::options_description desc() override;
  virtual void print(std::ostream& out) const override;
  virtual bool empty_config() const override { return false; }

  void init() override;
  virtual std::string name() const override { return "routing"; }
  virtual std::vector<MsgContent> subscriptions() const override {
    return {MsgContent_RoutingRequest};
  }
  virtual void on_msg(motis::module::msg_ptr, motis::module::sid,
                      motis::module::callback) override;

private:
  typedef std::function<void(arrival, boost::system::error_code)> path_el_cb;

  void read_path_element(StationPathElement const* el, path_el_cb);

  void handle_station_guess(motis::module::msg_ptr, boost::system::error_code,
                            path_el_cb);

  int max_label_count_;
  std::unique_ptr<memory_manager<label>> label_store_;
};

}  // namespace routing
}  // namespace motis
