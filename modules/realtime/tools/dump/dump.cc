#include <ctime>
#include <iostream>
#include <string>
#include <fstream>

#include "boost/date_time/posix_time/posix_time.hpp"
#include "boost/program_options.hpp"

#include "conf/options_parser.h"

#include "motis/realtime/database.h"
#include "motis/realtime/opt_time.h"

#define DB_SERVER "realtime.db_server"
#define DB_NAME "realtime.db_name"
#define DB_USER "realtime.db_user"
#define DB_PASSWORD "realtime.db_password"
#define FROM_TIME "realtime.from"
#define TO_TIME "realtime.to"
#define INTERVAL "realtime.interval"
#define OUTPUT "output,o"

namespace po = boost::program_options;
namespace rt = motis::realtime;

class dump_settings : public conf::configuration {
public:
  dump_settings() : interval_(30), output_("messages.txt") {}

  boost::program_options::options_description desc() {
    po::options_description desc("callback options");
    desc.add_options()(DB_SERVER, po::value<std::string>(&db_server_)
                                      ->default_value(db_server_),
                       "MySQL host (e.g. localhost or 127.0.0.1)")(
        DB_NAME, po::value<std::string>(&db_name_)->default_value(db_name_),
        "MySQL database name (e.g. risr)")(
        DB_USER, po::value<std::string>(&db_user_)->default_value(db_user_),
        "MySQL user")(
        DB_PASSWORD,
        po::value<std::string>(&db_password_)->default_value(db_password_),
        "MySQL password")(FROM_TIME, po::value<opt_time>(&from_time_),
                          "Load messages from " FROM_TIME " to " TO_TIME)(
        TO_TIME, po::value<opt_time>(&to_time_),
        "Load messages from " FROM_TIME
        " to " TO_TIME)(INTERVAL, po::value<unsigned>(&interval_),
                        "How many minutes should be loaded per update")(
        OUTPUT, po::value<std::string>(&output_)->default_value(output_),
        "Output filename");
    return desc;
  }

  void print(std::ostream& out) const {
    out << "  " << DB_SERVER << ": " << db_server_ << "\n"
        << "  " << DB_NAME << ": " << db_name_ << "\n"
        << "  " << DB_USER << ": " << db_user_ << "\n"
        << "  " << DB_PASSWORD << ": " << db_password_ << "\n"
        << "  " << FROM_TIME << ": " << from_time_ << "\n"
        << "  " << TO_TIME << ": " << to_time_ << "\n"
        << "  " << INTERVAL << ": " << interval_ << "\n"
        << "  " << OUTPUT << ": " << output_;
  }

  std::string db_server_, db_name_, db_user_, db_password_;
  opt_time from_time_, to_time_;
  unsigned interval_;
  std::string output_;
};

int main(int argc, char* argv[]) {
  dump_settings opt;
  conf::options_parser parser({&opt});
  parser.read_command_line_args(argc, argv);

  std::cout << "\n\tMOTIS realtime dump\n\n";

  if (parser.help()) {
    parser.print_help(std::cout);
    return 0;
  } else if (parser.version()) {
    return 0;
  }

  parser.read_configuration_file();

  parser.print_unrecognized(std::cout);
  parser.print_used(std::cout);

  std::time_t start_time = opt.from_time_;
  std::time_t end_time = opt.to_time_;
  unsigned interval = opt.interval_;

  if (opt.db_name_.empty() || opt.db_server_.empty() || opt.db_user_.empty()) {
    std::cerr << "Missing database parameters" << std::endl;
    return 2;
  } else if (start_time == 0) {
    std::cerr << "Missing --" << FROM_TIME << ", --" << TO_TIME << std::endl;
    return 3;
  }

  rt::delay_database db(opt.db_name_, opt.db_server_, opt.db_user_,
                        opt.db_password_);

  const char* time_format = "%a %d.%m.%Y (%j) %H:%M";

  if (end_time == 0) {
    end_time = start_time + 24 * 60 * 60;
  }

  std::cout << "Connecting to DB..." << std::endl;
  if (!db.connect()) {
    std::cout << "Connection failed!" << std::endl;
    return 1;
  }

  std::ofstream f(opt.output_);

  while (start_time < end_time) {
    std::time_t to = std::min(start_time + interval * 60, end_time);
    std::cout << "Loading messages from " << start_time << " ("
              << std::put_time(std::localtime(&start_time), time_format)
              << ") to " << to << " ("
              << std::put_time(std::localtime(&to), time_format) << ")..."
              << std::endl;

    f << db.get_messages(start_time, to);

    start_time = to;
  }

  std::cout << "Messages saved to " << opt.output_ << std::endl;

  return 0;
}
