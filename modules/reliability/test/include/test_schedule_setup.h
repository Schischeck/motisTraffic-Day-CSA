#pragma once

#include <string>

#include "gtest/gtest.h"

#include "motis/bootstrap/motis_instance_helper.h"

#include "motis/core/schedule/schedule.h"
#include "motis/loader/loader.h"

#include "motis/reliability/context.h"

namespace motis {
namespace reliability {

class test_schedule_setup : public ::testing::Test {
protected:
  test_schedule_setup(std::string schedule_name, std::time_t schedule_begin,
                      std::time_t schedule_end)
      : schedule_path_(std::move(schedule_name)),
        schedule_begin_(schedule_begin),
        schedule_end_(schedule_end) {}

  virtual void SetUp() override {
    schedule_ = loader::load_schedule(schedule_path_, true, schedule_begin_,
                                      schedule_end_);
  }

public:
  schedule_ptr schedule_;

private:
  std::string schedule_path_;
  std::time_t schedule_begin_, schedule_end_;
};

class test_motis_setup : public ::testing::Test {
protected:
  test_motis_setup(std::string schedule_path, std::string schedule_begin)
      : schedule_path_(schedule_path), schedule_begin_(schedule_begin) {}

  virtual void SetUp() override {
    std::vector<std::string> modules = {"reliability", "routing"};
    motis_instance_ = std::move(bootstrap::launch_motis(
        schedule_path_, schedule_begin_, modules, true));
    reliability_context_ = std::unique_ptr<motis::reliability::context>(
        new motis::reliability::context(
            get_schedule(),
            get_reliability_module().precomputed_distributions(),
            get_reliability_module().s_t_distributions()));
  }

public:
  std::unique_ptr<bootstrap::motis_instance> motis_instance_;

  schedule const& get_schedule() const { return *motis_instance_->schedule_; }
  reliability& get_reliability_module() {
    auto it = std::find_if(motis_instance_->modules_.begin(),
                           motis_instance_->modules_.end(),
                           [](std::unique_ptr<motis::module::module> const& m) {
                             return m->name() == "reliability";
                           });
    return *((reliability*)it->get());
  }

  std::unique_ptr<motis::reliability::context> reliability_context_;

private:
  std::string schedule_path_, schedule_begin_;
};

struct schedule_station {
  std::string name;
  std::string eva;
};

}  // namespace reliability
}  // namespace motis
