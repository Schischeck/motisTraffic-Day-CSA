namespace motis {
namespace loader {
namespace gtfs {

// AGENCIES
static char const* example_agencies_file_content =
    R"(agency_id,agency_name,agency_url,agency_timezone,agency_phone,agency_lang
FunBus,The Fun Bus,http://www.thefunbus.org,America/Los_Angeles,(310) 555-0222,en)";

// CALENDAR DATE
static char const* example_calendar_date_file_content =
    R"(service_id,date,exception_type
WD,20060703,2
WE,20060703,1
WD,20060704,2
WE,20060704,1)";

static char const* berlin_calendar_date_file_content =
    R"(service_id,date,exception_type
000001,20150409,1
000001,20150410,1
000001,20150411,1
000001,20150412,1
000001,20150705,1
000001,20150706,1
000001,20150707,1
000001,20150708,1
000001,20150709,1
000001,20150710,1
000001,20150711,1
000001,20150712,1
000001,20150713,1
000001,20150714,1
000002,20150715,1
000002,20150716,1
000002,20150717,1)";

// CALENDAR
static char const* example_calendar_file_content =
    R"(service_id,monday,tuesday,wednesday,thursday,friday,saturday,sunday,start_date,end_date
WE,0,0,0,0,0,1,1,20060701,20060731
WD,1,1,1,1,1,0,0,20060701,20060731)";

static char const* berlin_calendar_file_content =
    R"(service_id,monday,tuesday,wednesday,thursday,friday,saturday,sunday,start_date,end_date
000001,0,0,0,0,0,0,0,20150409,20151212
000002,0,0,0,0,0,0,0,20150409,20151212
000856,0,0,0,0,0,0,0,20150409,20151212
000861,0,0,0,0,0,0,0,20150409,20151212)";

// AGENCY
static char const* berlin_agencies_file_content =
    R"(agency_id,agency_name,agency_url,agency_timezone,agency_lang,agency_phone
ANG---,Günter Anger Güterverkehrs GmbH & Co. Omnibusvermietung KG,http://www.anger-busvermietung.de,Europe/Berlin,de,033208 22010
BMO---,Busverkehr Märkisch-Oderland GmbH,http://www.busmol.de,Europe/Berlin,de,03341 478383
N04---,DB Regio AG,http://www.bahn.de/brandenburg,Europe/Berlin,de,0331 2356881
BON---,Busverkehr Oder-Spree GmbH,http://www.bos-fw.de,Europe/Berlin,de,03361 556133)";

// ROUTE
static char const* example_routes_file_content =
    R"(route_id,route_short_name,route_long_name,route_desc,route_type
A,17,Mission,"The ""A"" route travels from lower Mission to Downtown.",3)";

static char const* berlin_routes_file_content =
    R"(route_id,agency_id,route_short_name,route_long_name,route_desc,route_type,route_url,route_color,route_text_color
1,ANG---,SXF2,,,700,http://www.vbb.de,,
10,BMO---,927,,,700,http://www.vbb.de,,
2,BEH---,548,,,700,http://www.vbb.de,,
809,N04---,,"Leisnig -- Leipzig, Hauptbahnhof",,100,http://www.vbb.de,,
81,BON---,2/412,,,700,http://www.vbb.de,,
810,N04---,,"S+U Lichtenberg Bhf (Berlin) -- Senftenberg, Bahnhof",,100,http://www.vbb.de,,
811,N04---,,"S+U Lichtenberg Bhf (Berlin) -- Altdöbern, Bahnhof",,100,http://www.vbb.de,,
812,N04---,RB14,,,100,http://www.vbb.de,,)";

// STOP
static char const* example_stops_file_content =
    R"(stop_id,stop_name,stop_desc,stop_lat,stop_lon,stop_url,location_type,parent_station
S1,Mission St. & Silver Ave.,The stop is located at the southwest corner of the intersection.,37.728631,-122.431282,,,
S2,Mission St. & Cortland Ave.,The stop is located 20 feet south of Mission St.,37.74103,-122.422482,,,
S3,Mission St. & 24th St.,The stop is located at the southwest corner of the intersection.,37.75223,-122.418581,,,
S4,Mission St. & 21st St.,The stop is located at the northwest corner of the intersection.,37.75713,-122.418982,,,
S5,Mission St. & 18th St.,The stop is located 25 feet west of 18th St.,37.761829,-122.419382,,,
S6,Mission St. & 15th St.,The stop is located 10 feet north of Mission St.,37.766629,-122.419782,,,
S7,24th St. Mission Station,,37.752240,-122.418450,,,S8
S8,24th St. Mission Station,,37.752240,-122.418450,http://www.bart.gov/stations/stationguide/stationoverview_24st.asp,1, )";

static char const* berlin_stops_file_content =
    R"(stop_id,stop_code,stop_name,stop_desc,stop_lat,stop_lon,zone_id,stop_url,location_type,parent_station
5100071,,Zbaszynek,,52.2425040,15.8180870,,,0,
9230005,,S Potsdam Hauptbahnhof Nord,,52.3927320,13.0668480,,,0,
9230006,,"Potsdam, Charlottenhof Bhf",,52.3930040,13.0362980,,,0,)";

// STOP_TIME
static char const* example_stop_times_file_content =
    R"(trip_id,arrival_time,departure_time,stop_id,stop_sequence,pickup_type,drop_off_type
AWE1,0:06:10,0:06:10,S1,1,0,0,0
AWE1,,,S2,2,0,1,3
AWE1,0:06:20,0:06:30,S3,3,0,0,0
AWE1,,,S5,4,0,0,0
AWE1,0:06:45,0:06:45,S6,5,0,0,0
AWD1,0:06:10,0:06:10,S1,1,0,0,0
AWD1,,,S2,2,0,0,0
AWD1,0:06:20,0:06:20,S3,3,0,0,0
AWD1,,,S4,4,0,0,0
AWD1,,,S5,5,0,0,0
AWD1,0:06:45,0:06:45,S6,6,0,0,0)";

static char const* berlin_stop_times_file_content =
    R"(trip_id,arrival_time,departure_time,stop_id,stop_sequence,stop_headsign,pickup_type,drop_off_type,shape_dist_traveled
1,04:45:00,04:46:00,9230999,1,,0,1,
1,04:51:00,04:51:00,9230400,2,,0,1,
1,04:59:00,04:59:00,9220019,3,,0,1,
1,05:06:00,05:06:00,9220070,4,,0,1,
1,05:11:00,05:11:00,9220114,5,,0,1,
1,05:13:00,05:13:00,9220001,6,,0,1,
1,05:32:00,05:32:00,9260024,7,,1,0,
2,05:35:00,05:35:00,9260024,1,,0,1,
2,05:54:00,05:54:00,9220001,2,,1,0,
2,05:56:00,05:56:00,9220114,3,,1,0,
2,06:02:00,06:03:00,9220070,4,,1,0,
2,06:08:00,06:08:00,9220019,5,,1,0,
2,06:15:00,06:15:00,9230400,6,,1,0,
2,06:22:00,06:22:00,9230999,7,,1,0,
3,06:53:00,06:53:00,9220186,1,,0,1,
3,06:55:00,06:55:00,9220222,2,,0,0,
3,06:58:00,06:58:00,9220196,3,,0,0,
3,07:00:00,07:00:00,9222132,4,,0,0,
3,07:02:00,07:02:00,9222131,5,,0,0,
3,07:05:00,07:05:00,9222151,6,,0,0,
3,07:07:00,07:07:00,9222280,7,,0,0,
3,07:09:00,07:09:00,9222085,8,,0,0,
3,07:11:00,07:11:00,9222088,9,,1,0,)";

// TRANSFERS
static char const* example_transfers_file_content =
    R"(from_stop_id,to_stop_id,transfer_type,min_transfer_time
S6,S7,2,300
S7,S6,3,
S23,S7,1,)";

static char const* berlin_transfers_file_content =
    R"(from_stop_id,to_stop_id,transfer_type,min_transfer_time,from_transfer_id,to_transfer_id
9003104,9003174,2,180,,
9003104,9003175,2,240,,
9003104,9003176,2,180,,
9003174,9003104,2,180,,
9003174,9003175,2,180,,)";

// TRIPS
static char const* example_trips_file_content =
    R"(route_id,service_id,trip_id,trip_headsign,block_id
A,WE,AWE1,Downtown,1
A,WE,AWD1,Downtown,2)";

static char const* berlin_trips_file_content =
    R"(route_id,service_id,trip_id,trip_headsign,trip_short_name,direction_id,block_id,shape_id
1,000856,1,Flughafen Schönefeld Terminal (Airport),,,1,
1,000856,2,S Potsdam Hauptbahnhof,,,2,
2,000861,3,"Golzow (PM), Schule",,,3,)";

}  // namespace gtfs
}  // namespace loader
}  // namespace motis