#include <atomic>
#include <functional>
#include <iostream>
#include <mutex>

#include "boost/thread.hpp"

#include "sqlite3.h"
#include "sqlpp11/sqlite3/sqlite3.h"
#include "sqlpp11/sqlpp11.h"

#include "conf/options_parser.h"
#include "conf/simple_config.h"

#include "motis/core/common/logging.h"
#include "motis/module/context/motis_parallel_for.h"
#include "motis/module/controller.h"
#include "motis/ris/database.h"
#include "motis/ris/detail/find_new_files.h"
#include "motis/ris/risml/risml_parser.h"
#include "motis/ris/zip_reader.h"

using namespace motis::logging;
using namespace motis::module;
using namespace motis::ris;
using namespace motis::ris::detail;
using namespace motis::ris::risml;
namespace sql = sqlpp::sqlite3;

struct settings : public conf::simple_config {
  settings(std::string input = "ris", std::string output = "ris.sqlite3")
      : simple_config("Settings") {
    string_param(input_, input, "input", "Folder with RISML zip files");
    string_param(output_, output, "output",
                 "Filename of the resulting sqlite database");
  }
  std::string input_, output_;
};

int main(int argc, char** argv) {
  LOG(info) << "Standalone RIS Loader\n";
  settings s{"ris", "ris.sqlite3"};

  try {
    conf::options_parser parser({&s});
    parser.read_command_line_args(argc, argv, false);
    if (parser.help()) {
      parser.print_help(std::cout);
      return 0;
    } else if (parser.version()) {
      return 0;
    }
    parser.print_used(std::cout);
  } catch (std::exception const& e) {
    std::cout << "options error: " << e.what() << "\n";
    return 1;
  }

  sql::connection_config conf;
  conf.path_to_database = s.output_;
  conf.flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE;
  conf.debug = false;

  auto db = std::make_unique<sql::connection>(conf);
  db_init(db);

  db->execute("DROP INDEX IF EXISTS ris_message_idx.ris_message;");
  db->execute("DROP INDEX IF EXISTS ris_message_timestamp_idx.ris_message;");

  auto known_files = db_get_files(db);
  auto new_files = find_new_files(s.input_, ".zip", known_files);

  LOG(info) << known_files.size() << " files already in the db, sparsing "
            << new_files.size() << " new zip files";

  std::mutex m;
  std::atomic_size_t count{0};

  controller c;
  c.scheduler_.enqueue(
      ctx_data(&c, nullptr),
      [&] {
        motis_parallel_for(new_files, [&](std::string const& new_file) {
          auto c = count.fetch_add(1);
          if (c % 100 == 0) {
            LOG(info) << "RIS parsing " << c << "/" << new_files.size();
          }

          std::vector<ris_message> parsed_messages;
          try {
            parsed_messages = parse_xmls(read_zip_file(new_file));
          } catch (std::exception const& e) {
            LOG(crit) << "bad zip file: " << new_file << " " << e.what();
            throw e;
          }

          std::lock_guard<std::mutex> lock(m);
          db_put_messages(db, new_file, parsed_messages);
        });
      },
      ctx::op_id(CTX_LOCATION));

  auto& ios = c.ios_;
  std::vector<boost::thread> threads(8);
  for (auto& thread : threads) {
    thread = boost::thread([&] { ios.run(); });
  }
  std::for_each(begin(threads), end(threads),
                [](boost::thread& t) { t.join(); });

  LOG(info) << "finished parsing. rebuild indices";
  db_init(db);
  LOG(info) << "done.";
  return 0;
}
