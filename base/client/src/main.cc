#include <iostream>

#include "snappy.h"

#include "motis/module/message.h"
#include "motis/client/motis_con.h"

using namespace motis::module;
using namespace motis::client;

int main(int argc, char* argv[]) {
  if (argc != 3) {
    printf("usage: %s {host} {port}\n", argv[0]);
    return 0;
  }

  std::string request;
  std::string line;
  do {
    request += line + "\n";
  } while (std::getline(std::cin, line));

  request = request.substr(1);
  auto req = make_msg(request);

  std::string buf;
  snappy::Compress(reinterpret_cast<char const*>(req->data()), req->size(),
                   &buf);

  std::string host = argv[1];
  std::string port = argv[2];
  boost::asio::io_service ios;
  auto con = std::make_shared<motis_con>(ios, host, port,
                                         boost::posix_time::seconds(30));
  con->query(buf, [&](net::tcp::tcp_ptr, std::string const& response,
                      boost::system::error_code ec) {
    if (ec) {
      printf("error: %s\n", ec.message().c_str());
      return;
    }

    std::string buf;
    snappy::Uncompress(static_cast<char const*>(response.data()),
                       response.size(), &buf);
    printf("%s\n", make_msg((void*)buf.data(), buf.size())->to_json().c_str());
  });

  ios.run();
}
