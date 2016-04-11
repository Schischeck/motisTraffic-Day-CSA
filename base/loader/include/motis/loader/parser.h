#pragma once

#include <string>
#include <vector>

#include "boost/filesystem/path.hpp"

#include "flatbuffers/flatbuffers.h"

#define SCHEDULE_FILE ("schedule.raw")

namespace motis {
namespace loader {

struct format_parser {
  virtual ~format_parser() = default;
  virtual bool applicable(boost::filesystem::path const& path) = 0;
  virtual std::vector<std::string> missing_files(
      boost::filesystem::path const& path) const = 0;
  virtual void parse(boost::filesystem::path const& path,
                     flatbuffers::FlatBufferBuilder&) = 0;
};

}  // namespace loader
}  // namespace motis
