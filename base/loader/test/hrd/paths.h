#pragma once

#include "boost/filesystem/path.hpp"

namespace motis {
namespace loader {
namespace hrd {

boost::filesystem::path const TEST_RESOURCES = "base/loader/test_resources/";
boost::filesystem::path const SCHEDULES = TEST_RESOURCES / "hrd_schedules";

}  // namespace hrd
}  // namespace loader
}  // namespace motis
