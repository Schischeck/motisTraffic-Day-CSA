#pragma once

#include <set>

#include "motis/module/module.h"
#include "motis/railviz/geometry.h"
#include "motis/railviz/context_manager.h"

namespace motis {
namespace railviz {

namespace bgi = boost::geometry::index;

typedef std::pair<geometry::box, unsigned int> rtree_value;
typedef bgi::rtree<rtree_value, bgi::rstar<16>> rtree_T;

struct railviz : public motis::module::module {
  railviz();

  virtual boost::program_options::options_description desc() override;
  virtual void print(std::ostream& out) const override;

  virtual std::string name() const override { return "railviz"; }
  virtual void init() override;
  virtual std::vector<json11::Json> on_msg(json11::Json const&,
                                           motis::module::sid) override;

  typedef std::function<std::vector<json11::Json>(
      railviz*, json11::Json const& msg)> operation;
  std::map<std::string, operation> ops_;

  //void get_trains_in_rect( std::vector<RTreeValue>&, RTreeBox rect );

  rtree_T rtree;
  std::vector<std::pair<int, const motis::edge*>> station_conns;
  ContextManager cmgr;
};

}  // namespace railviz
}  // namespace motis
