#pragma once

#include <cinttypes>
#include <map>

#include "flatbuffers/flatbuffers.h"

#include "motis/loader/loaded_file.h"
#include "motis/loader/parsers/hrd/providers_parser.h"
#include "motis/schedule-format/Provider_generated.h"

namespace motis {
namespace loader {
namespace hrd {

struct providers_translator {
  providers_translator(std::map<uint64_t, provider_info> const& providers,
                       flatbuffers::FlatBufferBuilder& builder)
      : providers_(providers), builder_(builder){};

  flatbuffers::Offset<Provider> get_or_create_provider(uint64_t admin);

  std::map<uint64_t, provider_info> const& providers_;
  std::map<uint64_t, flatbuffers::Offset<Provider>> fbs_providers_;
  flatbuffers::FlatBufferBuilder& builder_;
};

}  // hrd
}  // loader
}  // motis
