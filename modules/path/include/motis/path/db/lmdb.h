#pragma once

#include <memory>

#include "motis/path/db/kv_database.h"

namespace motis {
namespace path {

struct lmdb_database : public kv_database {
  explicit lmdb_database(std::string path,
                         size_t max_db_size = 10485760 /* 10M */);
  ~lmdb_database() override;

  lmdb_database(lmdb_database const&) = delete;
  lmdb_database(lmdb_database&&) = default;
  lmdb_database& operator=(lmdb_database const&) = delete;
  lmdb_database& operator=(lmdb_database&&) = default;

  void put(std::string const& k, std::string const& v) override;
  std::string get(std::string const& k) const override;
  boost::optional<std::string> try_get(std::string const& k) const override;

private:
  struct impl;
  std::unique_ptr<impl> impl_;
};

}  // namespace path
}  // namespace motis
