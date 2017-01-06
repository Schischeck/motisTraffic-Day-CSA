#pragma once

#include <map>

#include "motis/path/db/kv_database.h"

namespace motis {
namespace path {

struct map_database : public kv_database {
  ~map_database() override = default;

  void put(std::string const& k, std::string const& v) override {
    db_.emplace(k, v);
  }

  std::string get(std::string const& k) const override {
    auto const& it = db_.find(k);
    return (it != end(db_)) ? it->second : "";
  }

  boost::optional<std::string> try_get(std::string const& k) const override {
    auto const& it = db_.find(k);
    return (it != end(db_)) ? it->second : boost::optional<std::string>{};
  }

  std::map<std::string, std::string> db_;
};

}  // namespace path
}  // namespace motis
