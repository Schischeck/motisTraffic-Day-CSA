#pragma once

#include <ctime>
#include <limits>
#include <memory>
#include <set>
#include <string>
#include <vector>

#include "sqlpp11/sqlite3/connection.h"

namespace motis {
namespace ris {

struct ris_message;

using db_ptr = std::unique_ptr<sqlpp::sqlite3::connection>;

constexpr auto kDBInvalidTimestamp = std::numeric_limits<std::time_t>::max();

db_ptr default_db();

void db_init(db_ptr const& db = default_db());

void db_put_messages(std::string const& filename,
                     std::vector<ris_message> const& msgs,
                     db_ptr const& db = default_db());

std::time_t db_get_forward_start_time(db_ptr const& db = default_db());

std::set<std::string> db_get_files(db_ptr const& db = default_db());

std::vector<std::basic_string<uint8_t>> db_get_messages(
    std::time_t from, std::time_t to, db_ptr const& db = default_db());

void db_clean_messages(std::time_t threshold, db_ptr const& db = default_db());

}  // namespace ris
}  // namespace motis
