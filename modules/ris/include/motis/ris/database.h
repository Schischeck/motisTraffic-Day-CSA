#pragma once

#include <ctime>
#include <set>
#include <string>
#include <vector>

#include "motis/ris/ris_message.h"

namespace motis {
namespace ris {

void db_init();

void db_put_messages(std::string const& filename,
                     std::vector<ris_message> const& msgs);

std::set<std::string> db_get_stored_files();

std::vector<std::basic_string<uint8_t>> db_get_messages(std::time_t from,
                                                        std::time_t to);

}  // namespace ris
}  // namespace motis
