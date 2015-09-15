#include "motis/loader/parsers/hrd/providers_translator.h"

#include "parser/util.h"
#include "motis/loader/parsers/hrd/files.h"
#include "motis/loader/util.h"

using namespace parser;
using namespace flatbuffers;

namespace motis {
namespace loader {
namespace hrd {

Offset<Provider> providers_translator::get_or_create_provider(uint64_t admin) {
  return get_or_create(fbs_providers_, admin, [&]() -> Offset<Provider> {
    auto it = providers_.find(admin);
    if (it == end(providers_)) {
      return 0;
    } else {
      return CreateProvider(
          builder_, to_fbs_string(builder_, it->second.short_name, ENCODING),
          to_fbs_string(builder_, it->second.long_name, ENCODING),
          to_fbs_string(builder_, it->second.full_name, ENCODING));
    }
  });
}

}  // hrd
}  // loader
}  // motis
