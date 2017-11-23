#pragma once
#pragma once

#include "motis/module/module.h"

namespace motis {
namespace ris {

struct ris : public motis::module::module {
  ris();
  ~ris() override;

  ris(ris const&) = delete;
  ris& operator=(ris const&) = delete;

  ris(ris&&) = delete;
  ris& operator=(ris&&) = delete;

  void init(motis::module::registry&) override;

private:
  struct impl;
  std::unique_ptr<impl> impl_;

  std::string db_path_;
  std::string input_folder_;
  time_t init_time_;
};

}  // namespace ris
}  // namespace motis
