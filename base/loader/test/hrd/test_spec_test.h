#pragma once

#include "parser/buffer.h"
#include "parser/cstr.h"
#include "parser/util.h"

#include "boost/filesystem/path.hpp"

#include "motis/loader/hrd/model/hrd_service.h"
#include "motis/loader/hrd/model/specification.h"
#include "motis/loader/loaded_file.h"
#include "motis/loader/util.h"

namespace motis {
namespace loader {
namespace hrd {

boost::filesystem::path const TEST_RESOURCES = "base/loader/test_resources/";
boost::filesystem::path const SCHEDULES = TEST_RESOURCES / "hrd_schedules";

struct test_spec {
  test_spec(boost::filesystem::path const& root, char const* filename)
      : lf_(root / filename) {}

  std::vector<specification> get_specs();
  std::vector<hrd_service> get_hrd_services();
  loaded_file lf_;
};

}  // namespace hrd
}  // namespace loader
}  // namespace motis
