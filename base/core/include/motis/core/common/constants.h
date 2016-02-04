#pragma once

namespace motis {

constexpr double WALK_SPEED = 1.5;  // m/s
constexpr int BIKE_SPEED = 15;  // m/s

constexpr int MAX_WALK_TIME = 10 * 60; // s
constexpr int MAX_BIKE_TIME = 30 * 60; // s

constexpr int MAX_WALK_DIST = MAX_WALK_TIME * WALK_SPEED; // m
constexpr int MAX_BIKE_DIST = MAX_BIKE_TIME * BIKE_SPEED; // m

constexpr int MAX_TRAVEL_TIME_HOURS = 24;
constexpr int MAX_TRAVEL_TIME_MINUTES = MAX_TRAVEL_TIME_HOURS * 60;
constexpr int MAX_TRAVEL_TIME_SECONDS = MAX_TRAVEL_TIME_MINUTES * 60;

}  // namespace motis
