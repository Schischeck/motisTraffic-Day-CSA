#include "motis/path/prepare/db_check.h"

#include <iostream>

#include "websocketpp/common/md5.hpp"

#include "parser/util.h"

#include "motis/module/message.h"

#include "motis/path/fbs/PathIndex_generated.h"

using websocketpp::md5::md5_hash_hex;

using namespace motis::module;

namespace motis {
namespace path {

using lookup_table = typed_flatbuffer<motis::path::PathLookup>;

void check_databases(kv_database const& expected, kv_database const& actual) {
  auto const e_lut_buf = expected.try_get("__index");
  verify(e_lut_buf, "cannot load expected index");

  auto const a_lut_buf = actual.try_get("__index");
  verify(a_lut_buf, "cannot load actual index");

  lookup_table e_lut(e_lut_buf->size(), e_lut_buf->data());
  lookup_table a_lut(a_lut_buf->size(), a_lut_buf->data());

  verify(e_lut.get()->indices()->size() == a_lut.get()->indices()->size(),
         "lut size mismatch");

  for (auto i = 0u; i < e_lut.get()->indices()->size(); ++i) {
    auto const e_idx = std::to_string(e_lut.get()->indices()->Get(i)->index());
    auto const a_idx = std::to_string(a_lut.get()->indices()->Get(i)->index());

    auto const e_buf = expected.try_get(e_idx);
    verify(e_buf, "missing expected db entry");

    auto const a_buf = actual.try_get(a_idx);
    verify(a_buf, "missing actual db entry");

    if (*e_buf != *a_buf) {
      std::cout << make_msg(e_buf->data(), e_buf->size())->to_json() << "\n";
      std::cout << " =================================== \n";
      std::cout << make_msg(a_buf->data(), a_buf->size())->to_json() << "\n";
      return;
    }
  }

  std::cout << "checked " << e_lut.get()->indices()->size() << " entries.\n";
}

}  // namespace path
}  // namespace motis
