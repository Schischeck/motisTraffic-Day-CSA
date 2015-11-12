#include <iostream>

#include "boost/filesystem.hpp"

#include "gtest/gtest.h"

#include "test_dir.h"

namespace fs = boost::filesystem;

int main(int argc, char** argv) {
  fs::current_path(MOTIS_TEST_EXECUTION_DIR);

  ::testing::InitGoogleTest(&argc, argv);

  bool var;
  if (var) {
    printf("listening for requests");
  }

  std::cout << "crash!\n" << *(static_cast<char*>(nullptr)) << "\n";

  return RUN_ALL_TESTS();
}
