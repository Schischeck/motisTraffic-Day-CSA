#pragma once

#include <ctime>
#include <memory>
#include <string>
#include <vector>

#include "motis/module/module.h"
#include "motis/protocol/Message_generated.h"

namespace motis {
namespace rt {

struct rt : public motis::module::module {
  rt() = default;
  virtual ~rt() = default;

  virtual boost::program_options::options_description desc() override;
  virtual void print(std::ostream&) const override {}

  virtual std::string name() const override { return "rt"; }
  virtual void init() override {}
  virtual std::vector<MsgContent> subscriptions() const override {
    return {MsgContent_RISBatch};
  }
  virtual void on_msg(motis::module::msg_ptr, motis::module::sid,
                      motis::module::callback) override;

  int successful_trip_lookups = 0;
};

}  // namespace rt
}  // namespace motis
