#pragma once

#include "motis/module/module.h"

namespace motis {
namespace railviz {

class train_retriever;

struct railviz : public motis::module::module {
  railviz();
  virtual ~railviz();

  virtual boost::program_options::options_description desc() override;
  virtual void print(std::ostream& out) const override;

  virtual std::string name() const override { return "railviz"; }
  virtual std::vector<MsgContent> subscriptions() const override {
    return {MsgContent_RailVizStationDetailRequest};
  }
  virtual void init() override;
  virtual void on_open(motis::module::sid) override;
  virtual void on_close(motis::module::sid) override;
  virtual void on_msg(motis::module::msg_ptr, motis::module::sid,
                      motis::module::callback) override;

private:
  void station_info(motis::module::msg_ptr msg, motis::module::callback cb);

  typedef std::function<void(motis::module::msg_ptr, motis::module::callback)>
      op;
  std::map<MsgContent, op> ops_;
  std::unique_ptr<train_retriever> train_retriever_;
};

}  // namespace railviz
}  // namespace motis
