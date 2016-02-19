#pragma once

#include <vector>

#include "motis/module/module.h"
#include "motis/lookup/station_geo_index.h"

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
    return {MsgContent_LookupGeoStationRequest,
            MsgContent_LookupBatchGeoStationRequest,
            MsgContent_LookupStationEventsRequest,
            MsgContent_LookupIdTrainRequest};
  }

  virtual void on_msg(motis::module::msg_ptr, motis::module::sid,
                      motis::module::callback) override;

  void lookup_station(LookupGeoStationRequest const*,
                      motis::module::callback) const;
  void lookup_stations(LookupBatchGeoStationRequest const*,
                       motis::module::callback) const;

  void lookup_station_events(LookupStationEventsRequest const*,
                             motis::module::callback);

  void lookup_id_train(LookupIdTrainRequest const*, motis::module::callback);

  std::unique_ptr<station_geo_index> geo_index_;
};

}  // namespace lookup
}  // namespace motis
