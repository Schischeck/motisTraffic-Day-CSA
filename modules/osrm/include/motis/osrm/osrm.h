#pragma once

#include <memory>

#include "motis/module/module.h"

#include "motis/protocol/Message_generated.h"

namespace motis {
namespace osrm {

struct osrm : public motis::module::module {
public:
  osrm();
  ~osrm();

  virtual boost::program_options::options_description desc() override;
  virtual void print(std::ostream& out) const override;
  virtual bool empty_config() const override { return true; }

  virtual std::string name() const override { return "osrm"; }
  virtual void init() override;
  virtual std::vector<MsgContent> subscriptions() const override {
    return {MsgContent_OSRMRoutingRequest};
  }
  virtual void on_msg(motis::module::msg_ptr, motis::module::sid,
                      motis::module::callback) override;

private:
  std::string path_;

  class impl;
  std::unique_ptr<impl> impl_;
};

}  // namespace osrm
}  // namespace motis
