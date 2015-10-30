#pragma once

#include <ctime>
#include <memory>
#include <set>
#include <string>
#include <vector>

#include "sqlpp11/sqlite3/connection.h"

namespace motis {
namespace ris {

struct ris_message;

using db_ptr = std::unique_ptr<sqlpp::sqlite3::connection>;
db_ptr default_db();

void db_init(db_ptr const& db = default_db());

void db_put_messages(std::string const& filename,
                     std::vector<ris_message> const& msgs,
                     db_ptr const& db = default_db());

std::set<std::string> db_get_files(db_ptr const& db = default_db());

std::vector<std::basic_string<uint8_t>> db_get_messages(
    std::time_t from, std::time_t to, db_ptr const& db = default_db());

}  // namespace ris
}  // namespace motis
