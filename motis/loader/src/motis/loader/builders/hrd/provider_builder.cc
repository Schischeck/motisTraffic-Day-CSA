#include "motis/loader/builders/hrd/provider_builder.h"

#include "motis/loader/util.h"
#include "motis/loader/parsers/hrd/files.h"

namespace motis {
namespace loader {
namespace hrd {

using namespace parser;
using namespace flatbuffers;

provider_builder::provider_builder(
    std::map<uint64_t, provider_info> hrd_providers)
    : hrd_providers_(std::move(hrd_providers)) {}

Offset<Provider> provider_builder::get_or_create_provider(
    uint64_t admin, FlatBufferBuilder& fbb) {
  return get_or_create(fbs_providers_, admin, [&]() -> Offset<Provider> {
    auto it = hrd_providers_.find(admin);
    if (it == end(hrd_providers_)) {
      return 0;
    } else {
      return CreateProvider(fbb,
                            to_fbs_string(fbb, it->second.short_name, ENCODING),
                            to_fbs_string(fbb, it->second.long_name, ENCODING),
                            to_fbs_string(fbb, it->second.full_name, ENCODING));
    }
  });
}

}  // hrd
}  // loader
}  // motis
