#pragma once

#include "motis/osrm/osrm.h"

#include "motis/module/module.h"

namespace motis {
namespace osrm {

struct osrm : public motis::module::module {
  virtual ~osrm() {}

  virtual boost::program_options::options_description desc() override;
  virtual void print(std::ostream& out) const override;
  virtual bool empty_config() const override { return true; }

  virtual std::string name() const override { return "osrm"; }
  virtual void init() override;
  virtual std::vector<MsgContent> subscriptions() const override { return {}; }
  virtual void on_msg(motis::module::msg_ptr, motis::module::sid,
                      motis::module::callback) override;

private:

};

}  // namespace osrm
}  // namespace motis
