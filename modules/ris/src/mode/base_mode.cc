#include "motis/ris/mode/base_mode.h"

#include <memory>

#include "sqlpp11/sqlite3/connection.h"

#include "motis/core/common/logging.h"
#include "motis/module/module.h"

#include "motis/ris/database.h"
#include "motis/ris/detail/find_new_files.h"
#include "motis/ris/ris.h"
#include "motis/ris/risml/risml_parser.h"
#include "motis/ris/zip_reader.h"

using namespace motis::module;
using namespace motis::logging;
using namespace motis::ris;
using namespace motis::ris::detail;
using namespace motis::ris::risml;

namespace motis {
namespace ris {
namespace mode {

void base_mode::init(registry& r) {
  r.register_op("/ris/init", [this]{ init_async(); });
}

void base_mode::init_async() {
  sqlpp::sqlite3::connection_config conf;
  conf.path_to_database = "ris.sqlite3";
  conf.flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE;
  conf.debug = false;

  db_ = std::make_unique<sqlpp::sqlite3::connection>(conf);
  db_init(db_);
  read_files_ = db_get_files(db_);

  scoped_timer timer("RIS parsing RISML zip files into database");
  auto new_files = find_new_files(conf_->input_folder_, ".zip", read_files_);

  int c = 0;
  for (auto const& new_file : new_files) {  // TODO parallelize this loop
    if (++c % 100 == 0) {
      LOG(info) << "RIS parsing " << c << "/" << new_files.size() << std::endl;
    }

    auto parsed = parse_xmls(read_zip_file(new_file));
    db_put_messages(new_file, std::move(parsed), db_);
  }
}

void base_mode::forward(std::time_t const new_time) {

}

}  // namespace mode
}  // namespace ris
}  // namespace motis
