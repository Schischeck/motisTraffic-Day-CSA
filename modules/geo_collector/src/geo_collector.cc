#include "motis/geo_collector/geo_collector.h"

#include "motis/db/prep_stmt.h"

#include <iostream>

using namespace motis::module;
using namespace motis::db;

namespace motis {
namespace geo_collector {

geo_collector::geo_collector() : module("GeoCollector", "geoc") {}

constexpr char kCreateTableSQL[] =
    "CREATE TABLE IF NOT EXISTS users (id SERIAL PRIMARY KEY, name text);";
using create_table = void_stmt<kCreateTableSQL>;

constexpr char kInsertUserSQL[] = "INSERT INTO users (name) VALUES ($1);";
using insert = in_stmt<kInsertUserSQL, char const*>;

constexpr char kSelectUserSQL[] = "SELECT id, name FROM users";
using select = out_stmt<kSelectUserSQL, int, std::string>;

void geo_collector::init(registry& r) {

  // auto handle = db::connect();
  // db::execute(handle,
  //             "CREATE TABLE IF NOT EXISTS users (id bigint PRIMARY KEY, "
  //             "name text);");
  // db::close(handle);

  std::cout << "launch" << std::endl;

  connection_handle conn("dbname = postgres");
  create_table::exec(conn);

  insert::exec(conn, "algolocal");

  auto res = select::exec(conn);
  while(res.has_next()) {
    auto tuple = res.next();
    std::cout << std::get<0>(tuple) << " : " << std::get<1>(tuple) << "\n";
  }


  r.register_op("/geo_collector/sign_up",
                [this](msg_ptr const& m) { return sign_up(m); });
  r.register_op("/geo_collector/submit_measurements",
                [this](msg_ptr const& m) { return submit_measurements(m); });
  r.register_op("/geo_collector/submit_journey",
                [this](msg_ptr const& m) { return submit_journey(m); });
}

msg_ptr geo_collector::sign_up(msg_ptr const&) {
  std::cout << "hit signup" << std::endl;

  // connection_handle conn("dbname = postgres");
  // create_table::exec(conn);

  // insert::exec(conn, {"algolocal"});

  return make_success_msg();
}
msg_ptr geo_collector::submit_measurements(msg_ptr const&) {
  return make_success_msg();
}
msg_ptr geo_collector::submit_journey(msg_ptr const&) {
  return make_success_msg();
}

}  // namespace geo_collector
}  // namespace motis
