#pragma once

#include <sstream>
#include <set>

#include "motis/module/module.h"
#include "motis/railviz/date_converter.h"
#include "motis/railviz/timetable_retriever.h"
#include "motis/railviz/webclient.h"

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
    return {MsgContent_RailViz_alltra_req,
            MsgContent_RailViz_station_detail_req,
            MsgContent_RailViz_route_at_time_req};
  }
  virtual void init() override;
  virtual void on_open(motis::module::sid) override;
  virtual void on_close(motis::module::sid) override;
  virtual void on_msg(motis::module::msg_ptr, motis::module::sid,
                      motis::module::callback) override;

  static std::vector<std::string> clasz_names;
 private:
  void init(motis::module::msg_ptr msg, webclient& client,
            motis::module::callback cb);
  void all_station(motis::module::msg_ptr msg, webclient& client,
                   motis::module::callback cb);
  void station_info(motis::module::msg_ptr msg, webclient& client,
                    motis::module::callback cb);
  void all_trains(motis::module::msg_ptr msg, webclient& client,
                  motis::module::callback cb);
  void route_at_time(motis::module::msg_ptr msg, webclient& client,
                      motis::module::callback cb);
  motis::module::msg_ptr make_route_at_time_msg(const motis::schedule&, const route& ) const;

  typedef std::function<
      void(motis::module::msg_ptr, webclient&, motis::module::callback)> op;

  std::map<MsgContent, op> ops_;
  std::map<motis::module::sid, webclient> clients_;
  date_converter date_converter_;
  std::unique_ptr<train_retriever> train_retriever_;
  timetable_retriever timetable_retriever_;
};

}  // namespace railviz
}  // namespace motis
