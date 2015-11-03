#pragma once

#include <string>

#include "gtest/gtest.h"

#include "motis/core/schedule/schedule.h"
#include "motis/loader/loader.h"

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
    schedule_ =
        loader::load_schedule(schedule_path_, schedule_begin_, schedule_end_);
  }

public:
  schedule_ptr schedule_;

private:
  std::string schedule_path_;
  std::time_t schedule_begin_, schedule_end_;
};

struct schedule_station {
  std::string name;
  std::string eva;
};

}  // namespace reliability
}  // namespace motis
