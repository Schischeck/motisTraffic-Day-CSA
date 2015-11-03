#pragma once

#include <ctime>
#include <memory>
#include <string>
#include <vector>

#include "boost/asio/io_service.hpp"

#include "motis/module/module.h"
#include "motis/realtime/realtime_schedule.h"
#include "motis/realtime/message_fetcher.h"
#include "motis/realtime/opt_time.h"
#include "motis/protocol/Message_generated.h"

namespace motis {
namespace realtime {

struct realtime : public motis::module::module {
  realtime();
  virtual ~realtime();

  virtual boost::program_options::options_description desc() override;
  virtual void print(std::ostream& out) const override;

  virtual std::string name() const override { return "realtime"; }
  virtual void init() override;
  virtual std::vector<MsgContent> subscriptions() const override {
    return {MsgContent_RealtimeTrainInfoRequest,
            MsgContent_RealtimeTrainInfoBatchRequest,
            MsgContent_RealtimeForwardTimeRequest,
            MsgContent_RealtimeCurrentTimeRequest};
  }
  virtual void on_msg(motis::module::msg_ptr, motis::module::sid,
                      motis::module::callback) override;

  std::unique_ptr<realtime_schedule> rts_;
  std::unique_ptr<message_fetcher> message_fetcher_;

// settings
#ifdef WITH_MYSQL
  std::string db_server_, db_name_, db_user_, db_password_;
#endif
  std::vector<uint32_t> track_trains_;
  std::vector<std::string> msg_files_;
  bool debug_;
  opt_time from_time_, to_time_;
  bool live_;
  bool manual_;
  unsigned db_fetch_size_;

private:
  void get_train_info(motis::module::msg_ptr msg, motis::module::callback cb);
  void get_batch_train_info(motis::module::msg_ptr msg,
                            motis::module::callback cb);
  void forward_time(motis::module::msg_ptr msg, motis::module::callback cb);
  void current_time(motis::module::msg_ptr msg, motis::module::callback cb);

  typedef std::function<void(motis::module::msg_ptr, motis::module::callback)>
      op;
  std::map<MsgContent, op> ops_;
};

}  // namespace realtime
}  // namespace motis
