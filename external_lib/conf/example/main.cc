#include <iostream>

#include "conf/configuration.h"
#include "conf/options_parser.h"

#include "boost/program_options.hpp"

#define HOST "listen.host"
#define PORT "listen.port"

namespace po = boost::program_options;

class listener_settings : public conf::configuration {
public:
  friend std::ostream& operator<<(std::ostream& out,
                                  listener_settings const& options);

  listener_settings(std::string default_host,
                    std::string default_port)
      : host(default_host),
        port(default_port) {
  }

  virtual ~listener_settings() { }

  virtual boost::program_options::options_description desc() override {
    po::options_description desc("Listener Options");
    desc.add_options()
      (HOST,
          po::value<std::string>(&host)->default_value(host),
          "host to listen on (e.g. 0.0.0.0 or 127.0.0.1)")
      (PORT,
          po::value<std::string>(&port)->default_value(port),
          "port to listen on (e.g. 8080)");
    return desc;
  }

  virtual void print(std::ostream& out) const override{
    out << "  " << HOST << ": " << host << "\n"
        << "  " << PORT << ": " << port;
  }

  std::string host, port;
};

std::ostream& operator<<(std::ostream& out, const listener_settings& options) {
  options.print(out);
  return out;
}

int main(int argc, char* argv[]) {
  listener_settings listener_opt("0.0.0.0", "8080");
  conf::options_parser parser({ &listener_opt });
  parser.read_command_line_args(argc, argv);

  if (parser.help()) {
    parser.print_help(std::cout);
    return 0;
  } else if (parser.version()) {
    return 0;
  }

  parser.read_configuration_file();

  parser.print_unrecognized(std::cout);
  parser.print_used(std::cout);

  std::cout << "Starting server to listen on "
            << listener_opt.host << ":" << listener_opt.port
            << "\n";
}