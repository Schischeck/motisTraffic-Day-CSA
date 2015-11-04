#pragma once

#include <cinttypes>
#include <map>

#include "motis/loader/parsers/hrd/providers_parser.h"
#include "motis/schedule-format/Provider_generated.h"

namespace motis {
namespace loader {
namespace hrd {

struct provider_builder {
  provider_builder(std::map<uint64_t, provider_info>);

  flatbuffers::Offset<Provider> get_or_create_provider(
      uint64_t, flatbuffers::FlatBufferBuilder&);

  std::map<uint64_t, provider_info> const hrd_providers_;
  std::map<uint64_t, flatbuffers::Offset<Provider>> fbs_providers_;
};

}  // hrd
}  // loader
}  // motis
