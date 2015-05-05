#ifndef TD_EXECUTION_CALLBACK_SETTINGS_H_
#define TD_EXECUTION_CALLBACK_SETTINGS_H_

#include <string>

#include "conf/configuration.h"

namespace td {
namespace execution {

class callback_settings : public conf::configuration {
public:
  callback_settings(bool default_enabled,
                    std::string default_host,
                    std::string default_port,
                    std::string default_id);

  virtual ~callback_settings() { }

  virtual boost::program_options::options_description desc() override;

  virtual void print(std::ostream& out) const override;

  bool enabled;
  std::string host, port, id;
};

}  // namespace execution
}  // namespace td

#endif  // TD_EXECUTION_CALLBACK_SETTINGS_H_
