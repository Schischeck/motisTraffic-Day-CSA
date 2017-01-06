#pragma once

#include <memory>

#include "motis/path/db/kv_database.h"

namespace motis {
namespace path {

struct rocksdb_database : public kv_database {
  explicit rocksdb_database(std::string path);
  ~rocksdb_database() override;

  void put(std::string const& k, std::string const& v) override;
  std::string get(std::string const& k) const override;
  boost::optional<std::string> try_get(std::string const& k) const override;

private:
  struct impl;
  std::unique_ptr<impl> impl_;
};

}  // namespace path
}  // namespace motis
