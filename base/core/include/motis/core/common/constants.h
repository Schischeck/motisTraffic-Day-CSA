#pragma once

namespace motis {

constexpr int WALK_SPEED = 1;  // m/s
constexpr int BIKE_SPEED = 15;  // m/s

constexpr int MAX_WALK_TIME = 10 * 60; // s
constexpr int MAX_BIKE_TIME = 30 * 60; // s

constexpr int MAX_WALK_DIST = MAX_WALK_TIME * WALK_SPEED; // m
constexpr int MAX_BIKE_DIST = MAX_BIKE_TIME * BIKE_SPEED; // m

}  // namespace motis
