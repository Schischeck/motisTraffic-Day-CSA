#include <atomic>
#include <deque>
#include <functional>
#include <iostream>
#include <mutex>
#include <thread>

#include "sqlite3.h"
#include "sqlpp11/sqlpp11.h"
#include "sqlpp11/sqlite3/sqlite3.h"

#include "boost/optional.hpp"
#include "boost/program_options.hpp"

#include "conf/configuration.h"
#include "conf/options_parser.h"

#include "motis/core/common/logging.h"
#include "motis/ris/detail/find_new_files.h"
#include "motis/ris/database.h"
#include "motis/ris/risml_parser.h"
#include "motis/ris/zip_reader.h"

using namespace motis::logging;
using namespace motis::ris::detail;
namespace po = boost::program_options;
namespace sql = sqlpp::sqlite3;

namespace motis {
namespace ris {

constexpr auto kInput = "input";
constexpr auto kOutput = "output";

struct settings : public conf::configuration {

  settings(std::string input, std::string output)
      : input(std::move(input)), output(std::move(output)) {}

  po::options_description desc() override {
    po::options_description desc("Settings");
    // clang-format off
  desc.add_options()
      (kInput, po::value<std::string>(&input)->default_value(input),
       "Folder with RISML zip files")
      (kOutput, po::value<std::string>(&output)->default_value(output), 
       "Filename of the resulting sqlite database");
    // clang-format on
    return desc;
  }

  void print(std::ostream& out) const override {
    out << "  " << kInput << ": " << input << "\n"
        << "  " << kOutput << ": " << output;
  }

  std::string input, output;
};

template <typename T>
class runner {
public:
  runner(std::vector<T> const& jobs) {
    for (auto& job : jobs) {
      jobs_.push_back(std::reference_wrapper<T const>(job));
    }
    count_ = jobs.size();
  }

  boost::optional<std::reference_wrapper<T const>> next() {
    std::lock_guard<std::mutex> lock(mutex_);
    boost::optional<std::reference_wrapper<T const>> opt;

    if (!jobs_.empty()) {
      opt = jobs_.front();
      jobs_.pop_front();
    }

    return opt;
  }

  void run(std::function<void(T const&)> runnable,
           std::string const& desc = "job", unsigned const mod = 1000) {
    std::atomic<unsigned> count(0);

    std::vector<std::thread> threads;
    for (unsigned i = 0; i < std::thread::hardware_concurrency(); ++i) {
      // for (unsigned i = 0; i < 1; ++i) {
      threads.emplace_back([this, &runnable, &count, &desc, mod]() {

        while (true) {
          auto job = next();
          if (job == boost::none) break;

          auto c = count.fetch_add(1);
          if (c % mod == 0) {
            LOG(info) << desc << " " << c << "/" << count_;
          }

          runnable(job->get());
        }

      });
    }
    for (auto& thread : threads) {
      thread.join();
    }
  }

private:
  std::deque<std::reference_wrapper<T const>> jobs_;
  unsigned count_;

  std::mutex mutex_;
};

template <typename T>
void run_parallel(std::string const& desc, std::vector<T> const& jobs,
                  unsigned const mod, std::function<void(T const&)> runnable) {
  runner<T> runner(jobs);
  runner.run(runnable, desc, mod);
}

}  // namsepace ris
}  // namespace motis

using namespace motis::ris;

int main(int argc, char** argv) {
  LOG(info) << "Standalone RIS Loader\n";
  settings s{"ris", "ris.sqlite"};

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
  conf.path_to_database = s.output;
  conf.flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE;
  conf.debug = false;

  db_ptr db(new sql::connection(conf));
  db_init(db);

  db->execute("DROP INDEX IF EXISTS ris_message_idx.ris_message;");
  db->execute("DROP INDEX IF EXISTS ris_message_timestamp_idx.ris_message;");

  auto known_files = db_get_files(db);
  auto new_files = find_new_files(s.input, ".zip", known_files);

  LOG(info) << known_files.size() << " files already in the db, sparsing " 
            << new_files.size() << " new zip files";
  std::mutex db_mutex;
  auto func = [&db, &db_mutex](std::string const& new_file) {
    std::vector<ris_message> parsed_messages;
    try {
      parsed_messages = parse_xmls(read_zip_file(new_file));
    } catch (std::exception const& e) {
      LOG(error) << "bad zip file: " << new_file << " " << e.what();
      throw e;
    }

    std::lock_guard<std::mutex> lock(db_mutex);
    db_put_messages(new_file, parsed_messages, db);
  };
  run_parallel<std::string>("parse_ris_file", new_files, 60, func);
  LOG(info) << "finished parsing. rebuild indices";
  db_init(db);
  LOG(info) << "done.";
  return 0;
}
