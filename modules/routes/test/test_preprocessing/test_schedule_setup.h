#pragma once

#include <string>
#include <utility>

#include "gtest/gtest.h"

#include "motis/core/schedule/schedule.h"
#include "motis/loader/loader.h"

namespace motis {
namespace routes {

class test_schedule_setup : public ::testing::Test {
protected:
  test_schedule_setup(std::string schedule_name, std::string schedule_begin)
      : schedule_path_(std::move(schedule_name)),
        schedule_begin_(std::move(schedule_begin)) {}

  void SetUp() override {
    std::string file =
        "modules/routes/test/test_preprocessing/test_osm/simple_graph.osm";
    osm_loader osm_loader(file, osm_nodes_, osm_routes_);
    osm_loader.load_osm();
    schedule_ = loader::load_schedule(
        {schedule_path_, schedule_begin_, 2, false, false, false, true});
  }

public:
  schedule_ptr schedule_;
  std::map<int64_t, osm_node> osm_nodes_;
  std::map<int64_t, osm_route> osm_routes_;
  std::map<int32_t, std::vector<int64_t>> stations_;

private:
  std::string schedule_path_;
  std::string schedule_begin_;
};

}  // namespace routes
}  // namespace motis
