#pragma once

#include <string>

#include "gtest/gtest.h"

#include "motis/test/motis_instance_test.h"

#include "motis/core/schedule/schedule.h"
#include "motis/loader/loader.h"

#include "motis/reliability/context.h"
#include "motis/reliability/reliability.h"

namespace motis {
namespace reliability {

class test_schedule_setup : public test::motis_instance_test {
protected:
  test_schedule_setup(std::string schedule_path, std::string schedule_begin)
      : motis_instance_test(
            {schedule_path, schedule_begin, 2, false, false, false, true},
            {""}) {}
};

class test_motis_setup : public test::motis_instance_test {
public:
  std::unique_ptr<motis::reliability::context> reliability_context_;

  schedule const& get_schedule() const { return sched(); }
  reliability& get_reliability_module() {
    return *get_module<reliability>("reliability");
  }

protected:
  explicit test_motis_setup(std::string const schedule_path,
                            std::string const schedule_begin,
                            bool const realtime = false,
                            bool const bikesharing = false,
                            std::string const bikesharing_path = "")
      : motis_instance_test(
            {schedule_path, schedule_begin, 2, false, false, false, true},
            get_modules(realtime, bikesharing),
            get_cmdline_opt(bikesharing, bikesharing_path)) {
    reliability_context_ = std::make_unique<motis::reliability::context>(
        sched(), get_reliability_module().precomputed_distributions(),
        get_reliability_module().s_t_distributions());
  }

private:
  static std::vector<std::string> get_modules(bool const realtime,
                                              bool const bikesharing) {
    std::vector<std::string> modules = {"reliability", "routing"};
    if (realtime) {
      modules.push_back("connectionchecker");
      modules.push_back("realtime");
      modules.push_back("ris");
    }
    if (bikesharing) {
      modules.push_back("bikesharing");
      modules.push_back("intermodal");
    }
    return modules;
  }

  static std::vector<std::string> get_cmdline_opt(
      bool const bikesharing, std::string const bikesharing_path) {
    std::vector<std::string> modules_cmdline_opt;
    if (bikesharing) {
      modules_cmdline_opt.push_back("--bikesharing.nextbike_path=" +
                                    bikesharing_path);
      modules_cmdline_opt.push_back("--bikesharing.database_path=:memory:");
    }
    return modules_cmdline_opt;
  }
};

struct schedule_station {
  char const* name_;
  char const* eva_;
};

}  // namespace reliability
}  // namespace motis
