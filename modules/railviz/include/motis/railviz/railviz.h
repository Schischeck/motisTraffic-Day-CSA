#pragma once

#include <set>

#include "motis/module/module.h"
#include "motis/railviz/webclient_context_manager.h"
#include "motis/railviz/train_query.h"

namespace motis {
namespace railviz {

class edge_geo_index;

struct railviz : public motis::module::module {
  railviz();
  ~railviz();

  virtual boost::program_options::options_description desc() override;
  virtual void print(std::ostream& out) const override;

  virtual std::string name() const override { return "railviz"; }
  virtual void init() override;
  virtual json11::Json on_msg(json11::Json const&, motis::module::sid) override;

  typedef std::function<json11::Json(railviz*, json11::Json const& msg)> op;
  std::map<std::string, op> ops_;
  std::unique_ptr<edge_geo_index> edge_geo_index_;
  date_converter date_converter_;
  webclient_context_manager cmgr;
};

}  // namespace railviz
}  // namespace motis
