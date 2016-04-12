#pragma once

#include <ctime>
#include <limits>
#include <memory>
#include <set>
#include <string>
#include <vector>

#include "sqlpp11/sqlite3/connection.h"

#include "motis/ris/blob.h"

namespace motis {
namespace ris {

struct ris_message;

using db_ptr = std::unique_ptr<sqlpp::sqlite3::connection>;

constexpr auto kDBInvalidTimestamp = std::numeric_limits<std::time_t>::max();

void db_init(db_ptr const& db);

void db_put_messages(db_ptr const& db, std::string const& filename,
                     std::vector<ris_message> const& msgs);

std::set<std::string> db_get_files(db_ptr const& db);

std::time_t db_get_forward_start_time(db_ptr const& db,
                                      std::time_t schedule_begin,
                                      std::time_t schedule_end);

std::vector<std::pair<std::time_t, blob>> db_get_messages(
    db_ptr const& db, std::time_t schedule_begin, std::time_t schedule_end,
    std::time_t batch_from, std::time_t batch_to);

void db_housekeeping(db_ptr const& db, std::time_t threshold);

}  // namespace ris
}  // namespace motis
