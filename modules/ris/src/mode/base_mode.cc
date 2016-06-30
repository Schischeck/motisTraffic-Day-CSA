#include "motis/ris/mode/base_mode.h"

#include <atomic>
#include <memory>

#include "sqlpp11/sqlite3/connection.h"

#include "motis/core/common/logging.h"
#include "motis/core/access/time_access.h"
#include "motis/module/context/get_schedule.h"
#include "motis/module/context/motis_parallel_for.h"
#include "motis/module/context/motis_publish.h"
#include "motis/module/module.h"

#include "motis/ris/database.h"
#include "motis/ris/detail/find_new_files.h"
#include "motis/ris/detail/forward_batched.h"
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
  r.subscribe_void("/init", [this] { init_async(); });
}

void base_mode::init_async() {
  sqlpp::sqlite3::connection_config conf;
  conf.path_to_database = conf_->database_file_;
  conf.flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE;
  conf.debug = false;

  db_ = std::make_unique<sqlpp::sqlite3::connection>(conf);
  db_init(db_);
  read_files_ = db_get_files(db_);

  scoped_timer timer("RIS parsing RISML zip files into database");
  auto new_files = find_new_files(conf_->input_folder_, ".zip", read_files_);

  std::mutex m;
  std::atomic_size_t count{0};
  motis_parallel_for(new_files, [&](std::string const& new_file) {
    auto c = count.fetch_add(1);
    if (c % 100 == 0) {
      LOG(info) << "RIS parsing " << c << "/" << new_files.size();
    }

    try {
      auto parsed = parse_xmls(read_zip_file(new_file));
      std::lock_guard<std::mutex> lock(m);
      db_put_messages(db_, new_file, std::move(parsed));
    } catch (std::exception const& e) {
      LOG(crit) << "bad zip file: " << e.what() << " (" << new_file << ")";
    }
  });
}

void base_mode::forward(std::time_t const new_time) {
  auto& sched = get_schedule();
  auto const now = sched.system_time_;
  auto const sched_begin = external_schedule_begin(sched);
  auto const sched_end = external_schedule_end(sched);

  system_time_forward(new_time, [&] {
    if (now == 0) {
      return forward_batched(sched_begin, sched_end, new_time, db_);
    } else {
      return forward_batched(sched_begin, sched_end, now, new_time, db_);
    }
  });
}

}  // namespace mode
}  // namespace ris
}  // namespace motis
