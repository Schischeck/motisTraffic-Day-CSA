#pragma once

#include <set>

#include "motis/module/module.h"
#include "motis/railviz/date_converter.h"
#include "motis/railviz/webclient_manager.h"
#include "motis/railviz/webclient.h"

namespace motis {
namespace railviz {

class train_retriever;

struct railviz : public motis::module::module {
  railviz();
  ~railviz();

  virtual boost::program_options::options_description desc() override;
  virtual void print(std::ostream& out) const override;

  virtual std::string name() const override { return "railviz"; }
  virtual void init() override;
  virtual void on_open(motis::module::sid) override;
  virtual void on_close(motis::module::sid) override;
  virtual json11::Json on_msg(json11::Json const&, motis::module::sid) override;

  typedef std::function<json11::Json(railviz*, webclient&, json11::Json const& msg)> op;
  std::map<std::string, op> ops_;
  webclient_manager webclient_manager_;
  date_converter date_converter_;
  std::unique_ptr<train_retriever> train_retriever_;
};

}  // namespace railviz
}  // namespace motis
