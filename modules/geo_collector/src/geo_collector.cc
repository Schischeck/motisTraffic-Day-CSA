#include "motis/geo_collector/geo_collector.h"

// #include "motis/db/prep_stmt.h"

#include <iostream>

#include "boost/tokenizer.hpp"

#include "pgdb_default_conn.h"

#include "pgdb/pgdb.h"

#include "motis/module/message.h"

#include "motis/geo_collector/auth_token_generator.h"

using namespace motis::module;
using namespace pgdb;

namespace motis {
namespace geo_collector {

constexpr char kCreateParticipants[] = R"sql(
CREATE TABLE IF NOT EXISTS participants (
  id BIGSERIAL PRIMARY KEY,
  auth_token text,
  -- registered
  device_type text,
  device_id text
);
)sql";
using create_participants_table = void_stmt<kCreateParticipants>;

constexpr char kInsertParticipant[] = R"sql(
INSERT INTO participants
  ( auth_token, device_type, device_id ) 
VALUES
  ( $1, $2, $3 )
RETURNING id;
)sql";
using insert_participant =
    prep_stmt<kInsertParticipant,
              std::tuple<std::string, std::string, std::string>,
              std::tuple<int>>;

constexpr char kCreateLocations[] = R"sql(
CREATE TABLE IF NOT EXISTS locations (
  id BIGSERIAL PRIMARY KEY,
  provider_name text,
  time timestamp,
  latitude double precision,
  longitude double precision,
  accuracy double precision,
  participant_id bigint,
  insert_time timestamp DEFAULT current_timestamp
);
)sql";
using create_locations_table = void_stmt<kCreateLocations>;

constexpr char kInsertLocations[] = R"sql(
INSERT INTO locations
  ( provider_name, time, latitude, longitude, accuracy, participant_id )
VALUES
  ( $1, to_timestamp($2), $3, $4, $5, $6 );
)sql";
using insert_location = in_stmt<kInsertLocations, std::string, std::time_t,
                                double, double, double, long>;

constexpr char kCreateCatchall[] = R"sql(
CREATE TABLE IF NOT EXISTS catchall (
  id BIGSERIAL PRIMARY KEY,
  message text,
  participant_id bigint,
  insert_time timestamp DEFAULT current_timestamp
);
)sql";
using create_catchall_table = void_stmt<kCreateCatchall>;

constexpr char kInsertCatchall[] =
    "INSERT INTO catchall (message, participant_id) VALUES ($1, $2);";
using insert_catchall = in_stmt<kInsertCatchall, std::string, long>;

constexpr char kCreateJourneys[] = R"sql(
CREATE TABLE IF NOT EXISTS journeys (
  id BIGSERIAL PRIMARY KEY,
  document text,
  participant_id bigint
);
)sql";
using create_journeys_table = void_stmt<kCreateJourneys>;

constexpr char kInsertJourneys[] =
    "INSERT INTO journeys(document, participant_id) VALUES($1, $2);";
using insert_journey = in_stmt<kInsertJourneys, std::string, long>;

geo_collector::geo_collector() : module("GeoCollector", "geoc") {
  string_param(conninfo_, PGDB_DEFAULT_CONN, "conninfo",
               "How to connect to a postgres database.");
}

void geo_collector::init(registry& r) {
  connection_handle conn(conninfo_);
  create_participants_table::exec(conn);
  create_locations_table::exec(conn);
  create_journeys_table::exec(conn);
  create_catchall_table::exec(conn);

  r.register_op("/geo_collector/sign_up",
                [this](msg_ptr const& m) { return sign_up(m); });
  r.register_op("/geo_collector/submit_measurements",
                [this](msg_ptr const& m) { return submit_measurements(m); });
  r.register_op("/geo_collector/submit_journey",
                [this](msg_ptr const& m) { return submit_journey(m); });
  r.register_op("/geo_collector/upload",
                [this](msg_ptr const& m) { return upload(m); });
}

msg_ptr geo_collector::sign_up(msg_ptr const& msg) {
  auto req = motis_content(GeoCollectorSignUpRequest, msg);
  connection_handle conn(conninfo_);

  auto&& token = generate_auth_token();
  auto id = single_val<insert_participant>(
      conn, token, req->device_type()->str(), req->device_id()->str());

  message_creator b;
  b.create_and_finish(
      MsgContent_GeoCollectorSignUpResponse,
      CreateGeoCollectorSignUpResponse(b, id, b.CreateString(token)).Union());
  return make_msg(b);
}

msg_ptr geo_collector::submit_measurements(msg_ptr const& msg) {
  auto req = motis_content(GeoCollectorSubmitMeasurementsRequest, msg);
  auto participant = req->participant();

  connection_handle conn(conninfo_);
  insert_location insert_loc(conn);
  insert_catchall insert_ca(conn);

  auto str = req->measurements()->str();
  using tokenizer = boost::tokenizer<boost::char_separator<char>>;
  tokenizer tok(str, boost::char_separator<char>("\n"));
  for (auto const& line : tok) {
    std::cout << "-- " << line << std::endl;
    if (line.size() == 0) {
      continue;
    }

    switch (line[0]) {
      case 'L': {
        std::istringstream in(line);
        in.exceptions(std::ios_base::failbit);

        char ignored;
        std::time_t time;
        std::string provider;
        double lat, lng, acc;

        try {
          in >> ignored >> time >> provider >> lat >> lng >> acc;
          insert_loc(provider, time, lat, lng, acc, participant);
        } catch (...) {
          continue;
        }
        break;
      }
      default: insert_ca(line, participant); break;
    }
  }
  return make_success_msg();
}

msg_ptr geo_collector::submit_journey(msg_ptr const& msg) {
  auto req = motis_content(GeoCollectorSubmitJourneyRequest, msg);

  connection_handle conn(conninfo_);
  insert_journey::exec(conn, req->journey()->str(), req->participant());

  return make_success_msg();
}

msg_ptr geo_collector::upload(msg_ptr const& msg) {
  auto req = motis_content(HTTPRequest, msg);

  std::cout << "REQ INC" << std::endl;
  std::cout << req->content()->str() << std::endl;
  return make_success_msg();
}

}  // namespace geo_collector
}  // namespace motis
