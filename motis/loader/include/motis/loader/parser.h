#pragma once

#include <string>

#define SCHEDULE_FILE ("schedule.bin")

namespace motis {
namespace loader {

struct parser {
  virtual ~parser(){};
  virtual bool applicable(std::string const& path) = 0;
  virtual void parse(std::string const& path) = 0;
};

}  // namespace loader
}  // namespace motis
