#ifndef TD_EXECUTION_PARALLEL_SETTINGS_H_
#define TD_EXECUTION_PARALLEL_SETTINGS_H_

#include "conf/configuration.h"

namespace td {
namespace execution {

class parallel_settings : public conf::configuration {
public:
  parallel_settings();
  virtual ~parallel_settings() { }
  virtual boost::program_options::options_description desc() override;
  virtual void print(std::ostream& out) const override;
  int num_threads;
};

}  // namespace execution
}  // namespace td

#endif  // TD_EXECUTION_PARALLEL_SETTINGS_H_
