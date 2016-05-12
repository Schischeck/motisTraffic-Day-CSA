#pragma once

#include <string>

#include "gtest/gtest.h"

#include "motis/test/motis_instance_helper.h"

#include "motis/core/schedule/schedule.h"
#include "motis/loader/loader.h"

#include "motis/reliability/context.h"
#include "motis/reliability/reliability.h"

namespace motis {
namespace reliability {

class test_schedule_setup : public ::testing::Test {
protected:
  test_schedule_setup(std::string schedule_name, std::string schedule_begin)
      : schedule_path_(std::move(schedule_name)),
        schedule_begin_(std::move(schedule_begin)) {}

  void SetUp() override {
    schedule_ = loader::load_schedule(
        {schedule_path_, false, true, false, false, schedule_begin_, 2});
  }

public:
  schedule_ptr schedule_;

private:
  std::string schedule_path_;
  std::string schedule_begin_;
};

class test_motis_setup : public ::testing::Test {
public:
  std::unique_ptr<bootstrap::motis_instance> motis_instance_;
  std::unique_ptr<motis::reliability::context> reliability_context_;

  schedule const& get_schedule() const { return *motis_instance_->schedule_; }
  reliability& get_reliability_module() {
    auto it = std::find_if(motis_instance_->modules_.begin(),
                           motis_instance_->modules_.end(),
                           [](std::unique_ptr<motis::module::module> const& m) {
                             return m->name() == "reliability";
                           });
    return *(reinterpret_cast<reliability*>(it->get()));
  }

protected:
  explicit test_motis_setup(std::string schedule_path,
                            std::string schedule_begin,
                            bool const realtime = false,
                            bool const bikesharing = false,
                            std::string bikesharing_path = "")
      : schedule_path_(std::move(schedule_path)),
        schedule_begin_(std::move(schedule_begin)),
        realtime_(realtime),
        bikesharing_(bikesharing),
        bikesharing_path_(std::move(bikesharing_path)) {}

  void SetUp() override {
    std::vector<std::string> modules = {"reliability", "routing"};
    std::vector<std::string> modules_cmdline_opt;
    if (realtime_) {
      modules.push_back("connectionchecker");
      modules.push_back("realtime");
      modules.push_back("ris");
    }
    if (bikesharing_) {
      modules.push_back("lookup");
      modules.push_back("bikesharing");
      modules_cmdline_opt.push_back("--bikesharing.nextbike_path=" +
                                    bikesharing_path_);
      modules_cmdline_opt.push_back("--bikesharing.database_path=:memory:");
    }
    motis_instance_ = test::launch_motis(schedule_path_, schedule_begin_,
                                         modules, modules_cmdline_opt);
    reliability_context_ = std::make_unique<motis::reliability::context>(
        get_schedule(), get_reliability_module().precomputed_distributions(),
        get_reliability_module().s_t_distributions());
  }

private:
  std::string schedule_path_, schedule_begin_;
  bool realtime_;
  bool bikesharing_;
  std::string bikesharing_path_;
};

struct schedule_station {
  char const* name_;
  char const* eva_;
};

}  // namespace reliability
}  // namespace motis
