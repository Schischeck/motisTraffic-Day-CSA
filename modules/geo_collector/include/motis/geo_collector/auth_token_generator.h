#pragma once

#include <random>

namespace motis {
namespace geo_collector {

std::string generate_auth_token() {
  std::random_device rd;
  std::mt19937 gen(rd());
  constexpr char charset[] =
      "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
  std::uniform_int_distribution<> dist(0, sizeof(charset) - 2);

  std::string token(64, ' ');
  std::generate(begin(token), end(token),
                [&charset, &dist, &gen] { return charset[dist(gen)]; });
  return token;
}

}  // namespace geo_collector
}  // namespace motis
