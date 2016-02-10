#pragma once

#include <vector>

#include "motis/module/module.h"

#include "motis/protocol/Message_generated.h"

namespace motis {
namespace lookup {

struct lookup : public motis::module::module {
  lookup();
  ~lookup();

  virtual boost::program_options::options_description desc() override;
  virtual void print(std::ostream& out) const override;
  virtual bool empty_config() const override { return true; }

  virtual std::string name() const override { return "lookup"; }
  virtual void init() override;
  virtual std::vector<MsgContent> subscriptions() const override {
    return {MsgContent_LookupGeoIndexRequest,
            MsgContent_LookupStationEventsRequest,
            MsgContent_LookupTrainRequest};
  }

  virtual void on_msg(motis::module::msg_ptr, motis::module::sid,
                      motis::module::callback) override;

  void lookup_station(LookupGeoIndexRequest const*, motis::module::callback);
  void lookup_station_events(LookupStationEventsRequest const*,
                             motis::module::callback);
  void lookup_train(LookupTrainRequest const*, motis::module::callback);

  struct impl;
  std::unique_ptr<impl> impl_;
};

}  // namespace lookup
}  // namespace motis
