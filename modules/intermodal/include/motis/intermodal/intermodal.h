#pragma once

#include <vector>

#include "motis/module/module.h"

namespace motis {
namespace intermodal {

struct intermodal : public motis::module::module {
  intermodal();
  ~intermodal();

  virtual boost::program_options::options_description desc() override;
  virtual void print(std::ostream& out) const override;
  virtual bool empty_config() const override { return true; }

  virtual std::string name() const override { return "intermodal"; }
  virtual void init() override;
  virtual std::vector<MsgContent> subscriptions() const override {
    return {MsgContent_IntermodalRoutingRequest,
            MsgContent_IntermodalGeoIndexRequest};
  }
  virtual void on_msg(motis::module::msg_ptr, motis::module::sid,
                      motis::module::callback) override;

  void handle_routing_request(IntermodalRoutingRequest const*,
                              motis::module::callback);
  void handle_geo_index_request(IntermodalGeoIndexRequest const*,
                                motis::module::callback);

  struct impl;
  std::unique_ptr<impl> impl_;
};

}  // namespace intermodal
}  // namespace motis
