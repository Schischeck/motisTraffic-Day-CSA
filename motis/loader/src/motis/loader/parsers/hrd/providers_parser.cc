#include "motis/loader/parsers/hrd/providers_parser.h"

#include "parser/arg_parser.h"
#include "parser/util.h"

#include "motis/core/common/logging.h"
#include "motis/loader/util.h"

using namespace parser;
using namespace motis::logging;

namespace motis {
namespace loader {
namespace hrd {

void verify_line_format(cstr line, char const* filename, int line_number) {
  // Verify that the provider number has 5 digits.
  auto provider_number = line.substr(0, size(5));
  verify(std::all_of(begin(provider_number), end(provider_number), [](char c) {
    return std::isdigit(c);
  }), "provider line format mismatch in %s:%d", filename, line_number);

  verify(line[6] == 'K' || line[6] == ':',
         "provider line format mismatch in %s:%d", filename, line_number);
}

std::string parse_name(cstr s) {
  bool start_is_quote = s[0] == '\'' || s[0] == '\"';
  char end = start_is_quote ? s[0] : ' ';
  int i = start_is_quote ? 1 : 0;
  while (s && s[i] != end) {
    ++i;
  }
  auto region = s.substr(start_is_quote ? 1 : 0, size(i - 1));
  return std::string(region.str, region.len);
}

provider_info read_provider_names(cstr line, char const* filename,
                                  int line_number) {
  provider_info info;

  int short_name = line.substr_offset(" K ");
  int long_name = line.substr_offset(" L ");
  int full_name = line.substr_offset(" V ");

  verify(short_name != -1 && long_name != -1 && full_name != -1,
         "provider line format mismatch in %s:%d", filename, line_number);

  info.short_name = parse_name(line.substr(short_name + 3));
  info.long_name = parse_name(line.substr(long_name + 3));
  info.full_name = parse_name(line.substr(full_name + 3));

  return info;
}

std::map<uint64_t, provider_info> parse_providers(loaded_file const& file) {
  scoped_timer timer("parsing providers");

  std::map<uint64_t, provider_info> providers;
  provider_info current_info;
  int previous_provider_number = 0;

  for_each_line_numbered(file.content(), [&](cstr line, int line_number) {
    auto provider_number = parse<int>(line.substr(0, size(5)));
    if (line[6] == 'K') {
      current_info = read_provider_names(line, file.name(), line_number);
      previous_provider_number = provider_number;
    } else {
      verify(previous_provider_number == provider_number,
             "provider line format mismatch in %s:%d", file.name(),
             line_number);
      for_each_token(line.substr(8), ' ', [&](cstr token) {
        providers[raw_to_int<uint64_t>(token)] = current_info;
      });
    }
  });

  return providers;
}

}  // loader
}  // motis
}  // hrd
