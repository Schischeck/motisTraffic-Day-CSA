#pragma once

#include <ctime>
#include <memory>
#include <string>
#include <vector>

#include "boost/asio/io_service.hpp"

#include "motis/module/module.h"
#include "motis/realtime/realtime_schedule.h"
#include "motis/protocol/Message_generated.h"

namespace motis {
namespace realtime {

struct realtime : public motis::module::module {
  realtime();
  virtual ~realtime() {}

  virtual boost::program_options::options_description desc() override;
  virtual void print(std::ostream& out) const override;

  virtual std::string name() const override { return "realtime"; }
  virtual void init() override;
  virtual std::vector<MsgContent> subscriptions() const override {
    return {MsgContent_RealtimeTrainInfoRequest,
            MsgContent_RealtimeTrainInfoBatchRequest,
            MsgContent_RealtimeCurrentTimeRequest,
            MsgContent_RealtimeDelayInfoRequest, MsgContent_RISBatch};
  }
  virtual void on_msg(motis::module::msg_ptr, motis::module::sid,
                      motis::module::callback) override;

  std::unique_ptr<realtime_schedule> rts_;

  // settings
  std::vector<uint32_t> track_trains_;
  bool debug_;

private:
  void get_train_info(motis::module::msg_ptr msg, motis::module::callback cb);
  void get_batch_train_info(motis::module::msg_ptr msg,
                            motis::module::callback cb);
  void get_current_time(motis::module::msg_ptr msg, motis::module::callback cb);
  void get_delay_infos(motis::module::msg_ptr msg, motis::module::callback cb);
  void handle_ris_msgs(motis::module::msg_ptr msg, motis::module::callback cb);

  typedef std::function<void(motis::module::msg_ptr, motis::module::callback)>
      op;
  std::map<MsgContent, op> ops_;
};

}  // namespace realtime
}  // namespace motis
