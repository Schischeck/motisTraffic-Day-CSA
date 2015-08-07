#pragma once

#include "boost/filesystem/path.hpp"

#define SCHEDULE_FILE ("schedule.bin")

namespace motis {
namespace loader {

struct parser {
  virtual ~parser(){};
  virtual bool applicable(boost::filesystem::path const& path) = 0;
  virtual void parse(boost::filesystem::path const& path) = 0;
};

}  // namespace loader
}  // namespace motis
