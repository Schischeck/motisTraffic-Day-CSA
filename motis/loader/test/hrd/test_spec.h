#pragma once

#include "parser/cstr.h"
#include "parser/buffer.h"
#include "parser/util.h"

#include "boost/filesystem/path.hpp"

#include "motis/loader/util.h"

#include "motis/loader/parsers/hrd/service/specification.h"
#include "motis/loader/parsers/hrd/service/hrd_service.h"

namespace motis {
namespace loader {
namespace hrd {

boost::filesystem::path const TEST_RESOURCES("motis/loader/test_resources/");

boost::filesystem::path const SCHEDULES = TEST_RESOURCES / "schedules";

struct test_spec {
  test_spec(boost::filesystem::path const& root, char const* filename)
      : buf_(load_file(root / filename)),
        lf_({filename, parser::cstr(buf_.data(), buf_.size())}) {}

  std::vector<specification> get_specs();
  std::vector<hrd_service> get_services();

  parser::buffer buf_;
  loaded_file lf_;
};

}  // hrd
}  // loader
}  // motis
