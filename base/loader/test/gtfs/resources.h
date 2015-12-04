#include "boost/filesystem/path.hpp"

namespace motis {
namespace loader {
namespace gtfs {

boost::filesystem::path const TEST_RESOURCES = "base/loader/test_resources/";
boost::filesystem::path const SCHEDULES = TEST_RESOURCES / "gtfs_schedules";

}  // namespace gtfs
}  // namespace loader
}  // namespace motis
