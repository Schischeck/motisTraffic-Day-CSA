#include "motis/geo_collector/geo_collector.h"

#include <ctime>
#include <fstream>
#include <iostream>

#include "boost/tokenizer.hpp"

#include "rapidjson/rapidjson_with_exception.h"
// #include "rapidjson/document.h"

#include "pgdb_default_conn.h"

#include "pgdb/pgdb.h"

#include "motis/module/message.h"

#include "motis/geo_collector/auth_token_generator.h"

using namespace rapidjson;
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
  ( time, latitude, longitude, accuracy, participant_id )
VALUES
  ( to_timestamp($1), $2, $3, $4, $5 );
)sql";
using insert_location =
    in_stmt<kInsertLocations, std::time_t, double, double, double, int>;

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
using insert_catchall = in_stmt<kInsertCatchall, std::string, int>;

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
using insert_journey = in_stmt<kInsertJourneys, std::string, int>;

geo_collector::geo_collector() : module("GeoCollector", "geoc") {
  string_param(conninfo_, PGDB_DEFAULT_CONN, "conninfo",
               "How to connect to a postgres database.");
}

constexpr char kCreateCrashReports[] = R"sql(
CREATE TABLE IF NOT EXISTS crash_reports (
  id BIGSERIAL PRIMARY KEY,
  insert_time timestamp DEFAULT current_timestamp,
  report jsonb
);
)sql";
using create_crash_reports_table = void_stmt<kCreateCrashReports>;

constexpr char kInsertCrashReport[] =
    "INSERT INTO crash_reports (report) VALUES ($1);";
using insert_crash_report = in_stmt<kInsertCrashReport, std::string>;

void geo_collector::init(registry& r) {
  connection_handle conn(conninfo_);
  create_participants_table::exec(conn);
  create_locations_table::exec(conn);
  create_journeys_table::exec(conn);
  create_catchall_table::exec(conn);
  create_crash_reports_table::exec(conn);

  r.register_op("/geo_collector/sign_up",
                [this](msg_ptr const& m) { return sign_up(m); });
  r.register_op("/geo_collector/submit_measurements",
                [this](msg_ptr const& m) { return submit_measurements(m); });
  r.register_op("/geo_collector/submit_journey",
                [this](msg_ptr const& m) { return submit_journey(m); });
  r.register_op("/geo_collector/submit_crash",
                [this](msg_ptr const& m) { return submit_crash(m); });
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
  auto req = motis_content(HTTPRequest, msg);

  auto it =
      std::find_if(std::begin(*req->headers()), std::end(*req->headers()),
                   [](auto&& h) { return h->name()->str() == "participant"; });
  if (it == std::end(*req->headers())) {
    std::cout << "participant missing" << std::endl;
    throw new std::runtime_error("participant missing");
  }
  auto participant_id = std::stoi(it->value()->str());

  auto content = req->content()->str();

  // static std::atomic_uint serial{0};  // multiple uploads per second
  // auto fname = std::to_string(std::time(nullptr)) + "-" +
  //              std::to_string(serial++) + ".json";
  // std::ofstream out(fname);
  // out << content;

  rapidjson::Document doc;
  bool failure = doc.Parse<0>(content.c_str()).HasParseError();
  if (failure) {
    std::cout << "invalid json" << std::endl;
    throw std::runtime_error("invalid json");
  }

  try {
    connection_handle conn(conninfo_);
    insert_location insert_loc(conn);
    auto const& items = doc["items"];
    for (Value::ConstValueIterator it = items.Begin(); it != items.End();
         ++it) {
      auto const& e = *it;
      insert_loc(e["timestamp"].GetInt64(),  //
                 e["latitude"].GetDouble(),
                 e["longitude"].GetDouble(),  //
                 e["accuracy"].GetDouble(),  //
                 participant_id);
    }

  } catch (rapidjson::rapidjson_error const& e) {
    std::cout << e.what() << std::endl;
  }

  return make_success_msg();
}
msg_ptr geo_collector::submit_journey(msg_ptr const& msg) {
  auto req = motis_content(GeoCollectorSubmitJourneyRequest, msg);

  connection_handle conn(conninfo_);
  insert_journey::exec(conn, req->journey()->str(), req->participant());

  return make_success_msg();
}

msg_ptr geo_collector::submit_crash(msg_ptr const& msg) {
  auto req = motis_content(HTTPRequest, msg);
  connection_handle conn(conninfo_);
  insert_crash_report::exec(conn, req->content()->str());
  return make_success_msg();
}

}  // namespace geo_collector
}  // namespace motis
