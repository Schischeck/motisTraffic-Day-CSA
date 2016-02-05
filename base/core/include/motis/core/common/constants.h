#pragma once

namespace motis {

constexpr auto WALK_SPEED = 1.5;  // m/s
constexpr auto BIKE_SPEED = 15.0 * (1000.0 / 3600.0);  // m/s (15 km/h)

constexpr auto MAX_WALK_TIME = 10 * 60; // s
constexpr auto MAX_BIKE_TIME = 30 * 60; // s

constexpr auto MAX_WALK_DIST = MAX_WALK_TIME * WALK_SPEED; // m
constexpr auto MAX_BIKE_DIST = MAX_BIKE_TIME * BIKE_SPEED; // m

constexpr auto MAX_TRAVEL_TIME_HOURS = 24;
constexpr auto MAX_TRAVEL_TIME_MINUTES = MAX_TRAVEL_TIME_HOURS * 60;
constexpr auto MAX_TRAVEL_TIME_SECONDS = MAX_TRAVEL_TIME_MINUTES * 60;

constexpr unsigned LATE_TAXI_BEGIN_TIME = 1260;  // minutes after midnight
constexpr unsigned LATE_TAXI_END_TIME = 180;  // minutes after midnight

}  // namespace motis
