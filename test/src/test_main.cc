#include <iostream>

#include "boost/filesystem.hpp"

#include "gtest/gtest.h"

#include "test_dir.h"

namespace fs = boost::filesystem;

int main(int argc, char** argv) {
  fs::current_path(MOTIS_TEST_EXECUTION_DIR);
  std::cout << "executing tests in " << fs::current_path() << std::endl;

  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
