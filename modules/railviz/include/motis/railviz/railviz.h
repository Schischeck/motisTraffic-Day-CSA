#pragma once

#include <sstream>
#include <set>

#include "motis/module/handler_functions.h"
#include "motis/module/module.h"
#include "motis/railviz/timetable_retriever.h"
#include "motis/railviz/webclient.h"
#include "motis/railviz/realtime_response.h"

namespace motis {
namespace railviz {

using namespace motis::module;

struct train_retriever;

struct railviz : public motis::module::module {
  railviz();
  virtual ~railviz();

  virtual boost::program_options::options_description desc() override;
  virtual void print(std::ostream& out) const override;

  virtual std::string name() const override { return "railviz"; }
  virtual std::vector<MsgContent> subscriptions() const override {
    return {MsgContent_RailVizAlltraReq, MsgContent_RailVizStationDetailReq,
            MsgContent_RailVizRouteAtTimeReq, MsgContent_RailVizFindTrain};
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
  motis::module::msg_ptr make_station_info_realtime_request(
      const timetable&) const;
  callback make_station_info_realtime_callback(int station_index,
                                               timetable const&, callback);

  void find_train(motis::module::msg_ptr msg, webclient& client,
                  motis::module::callback cb);

  void all_trains(motis::module::msg_ptr msg, webclient& client,
                  motis::module::callback cb);
  motis::module::msg_ptr make_all_trains_realtime_request(
      std::vector<std::pair<light_connection const*, edge const*>> const&)
      const;
  callback make_all_trains_realtime_callback(
      std::vector<std::pair<light_connection const*, edge const*>> const&, bool,
      callback);

  void route_at_time(motis::module::msg_ptr msg, webclient& client,
                     motis::module::callback cb);
  motis::module::msg_ptr make_route_at_time_msg(const motis::schedule&,
                                                const route&) const;
  callback make_route_at_time_realtime_callback(const route&, callback);

  typedef std::function<void(motis::module::msg_ptr, webclient&,
                             motis::module::callback)> op;

  std::time_t schedule_begin_;
  std::map<MsgContent, op> ops_;
  std::map<motis::module::sid, webclient> clients_;
  std::unique_ptr<train_retriever> train_retriever_;
  timetable_retriever timetable_retriever_;
};

}  // namespace railviz
}  // namespace motis
