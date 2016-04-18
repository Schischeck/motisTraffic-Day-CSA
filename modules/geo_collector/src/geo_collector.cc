#include "motis/geo_collector/geo_collector.h"

// #include "motis/db/prep_stmt.h"

#include <iostream>

#include "pgdb/pgdb.h"

#include "motis/module/message.h"

#include "motis/geo_collector/auth_token_generator.h"

using namespace motis::module;
using namespace pgdb;

namespace motis {
namespace geo_collector {

geo_collector::geo_collector() : module("GeoCollector", "geoc") {}

constexpr char kCreateParticipants[] = R"sql(
CREATE TABLE IF NOT EXISTS participants (
  id SERIAL PRIMARY KEY,
  auth_token text,
  -- registered
  device_type text,
  mac_address text
);
)sql";
using create_participants_table = void_stmt<kCreateParticipants>;

constexpr char kInsertParticipant[] = R"sql(
INSERT INTO participants
  ( auth_token, device_type, mac_address ) 
VALUES
  ( $1, $2, $3 )
RETURNING id;
)sql";
using insert_participant =
    prep_stmt<kInsertParticipant,
              std::tuple<std::string, std::string, std::string>,
              std::tuple<int>>;

void geo_collector::init(registry& r) {
  connection_handle conn("dbname=postgres");
  create_participants_table::exec(conn);

  // auto handle = db::connect();
  // db::execute(handle,
  //             "CREATE TABLE IF NOT EXISTS users (id bigint PRIMARY KEY, "
  //             "name text);");
  // db::close(handle);

  // std::cout << "launch" << std::endl;

  // connection_handle conn("dbname = postgres");
  // create_table::exec(conn);

  // insert::exec(conn, "algolocal");

  // auto res = select::exec(conn);
  // while(res.has_next()) {
  //   auto tuple = res.next();
  //   std::cout << std::get<0>(tuple) << " : " << std::get<1>(tuple) << "\n";
  // }

  r.register_op("/geo_collector/sign_up",
                [this](msg_ptr const& m) { return sign_up(m); });
  r.register_op("/geo_collector/submit_measurements",
                [this](msg_ptr const& m) { return submit_measurements(m); });
  r.register_op("/geo_collector/submit_journey",
                [this](msg_ptr const& m) { return submit_journey(m); });
}

msg_ptr geo_collector::sign_up(msg_ptr const& msg) {
  auto req = motis_content(GeoCollectorSignUpRequest, msg);

  connection_handle conn("dbname=postgres");

  auto&& token = generate_auth_token();
  auto id = single_val<insert_participant>(
      conn, token, 
      req->device_type()->str(), 
      req->mac_address()->str());

  message_creator b;
  b.create_and_finish(
      MsgContent_GeoCollectorSignUpResponse,
      CreateGeoCollectorSignUpResponse(b, id, b.CreateString(token)).Union());
  return make_msg(b);
}
msg_ptr geo_collector::submit_measurements(msg_ptr const&) {
  return make_success_msg();
}
msg_ptr geo_collector::submit_journey(msg_ptr const&) {
  return make_success_msg();
}

}  // namespace geo_collector
}  // namespace motis
