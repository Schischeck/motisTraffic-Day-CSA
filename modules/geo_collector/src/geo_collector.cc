#include "motis/geo_collector/geo_collector.h"

// #include "motis/db/prep_stmt.h"

#include <iostream>

#include "parser/csv.h"
#include "pgdb/pgdb.h"

#include "motis/module/message.h"

#include "motis/geo_collector/auth_token_generator.h"

using namespace motis::module;
using namespace parser;
using namespace pgdb;

namespace motis {
namespace geo_collector {

geo_collector::geo_collector() : module("GeoCollector", "geoc") {}

constexpr char kCreateParticipants[] = R"sql(
CREATE TABLE IF NOT EXISTS participants (
  id BIGSERIAL PRIMARY KEY,
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

constexpr char kCreateLocations[] = R"sql(
CREATE TABLE IF NOT EXISTS locations (
  id BIGSERIAL PRIMARY KEY,
  provider_name text,
  time timestamp,
  latitude double precision,
  longitude double precision,
  accuracy double precision,
  participant_id bigint
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

void geo_collector::init(registry& r) {
  connection_handle conn("dbname=postgres");
  create_participants_table::exec(conn);
  create_locations_table::exec(conn);
  create_journeys_table::exec(conn);

  r.register_op("/geo_collector/sign_up",
                [this](msg_ptr const& m) { return sign_up(m); });
  r.register_op("/geo_collector/submit_locations",
                [this](msg_ptr const& m) { return submit_locations(m); });
  r.register_op("/geo_collector/submit_journey",
                [this](msg_ptr const& m) { return submit_journey(m); });
}

msg_ptr geo_collector::sign_up(msg_ptr const& msg) {
  auto req = motis_content(GeoCollectorSignUpRequest, msg);
  connection_handle conn("dbname=postgres");

  auto&& token = generate_auth_token();
  auto id = single_val<insert_participant>(
      conn, token, req->device_type()->str(), req->mac_address()->str());

  message_creator b;
  b.create_and_finish(
      MsgContent_GeoCollectorSignUpResponse,
      CreateGeoCollectorSignUpResponse(b, id, b.CreateString(token)).Union());
  return make_msg(b);
}

using location = std::tuple<long, double, double, double>;
enum { time, lat, lng, acc };
static const column_mapping<location> columns = {{"time", "lat", "lng", "acc"}};

msg_ptr geo_collector::submit_locations(msg_ptr const& msg) {
  auto req = motis_content(GeoCollectorSubmitLocationsRequest, msg);

  connection_handle conn("dbname=postgres");
  insert_location insert(conn);

  auto participant = req->participant();

  for (auto const& p : *req->providers()) {
    auto p_name = p->name()->str();
    for (auto const& l :
         read<location, '\t'>(p->locations()->c_str(), columns)) {
      using std::get;
      insert(p_name, get<time>(l), get<lat>(l), get<lng>(l), get<acc>(l),
             participant);
    }
  }
  return make_success_msg();
}

msg_ptr geo_collector::submit_journey(msg_ptr const&msg ) {
  auto req = motis_content(GeoCollectorSubmitJourneyRequest, msg);

  connection_handle conn ("dbname=postgres");
  insert_journey::exec(conn, req->journey()->str(), req->participant());

  return make_success_msg();
}

}  // namespace geo_collector
}  // namespace motis
