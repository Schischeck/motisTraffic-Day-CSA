#pragma once

#include "motis/module/module.h"

namespace motis {
namespace railviz {

class train_retriever;

struct railviz : public motis::module::module {
  railviz();
  ~railviz();

  virtual boost::program_options::options_description desc() override;
  virtual void print(std::ostream& out) const override;

  virtual std::string name() const override { return "railviz"; }
  virtual std::vector<MsgContent> subscriptions() const {
    return {MsgContent_RailVizStationDetailRequest};
  }
  virtual void init() override;
  virtual void on_open(motis::module::sid) override;
  virtual void on_close(motis::module::sid) override;
  virtual motis::module::msg_ptr on_msg(motis::module::msg_ptr const&,
                                        motis::module::sid) override;

  typedef std::function<motis::module::msg_ptr(railviz*,
                                               motis::module::msg_ptr)> op;
  std::map<MsgContent, op> ops_;
  std::unique_ptr<train_retriever> train_retriever_;
};

}  // namespace railviz
}  // namespace motis
