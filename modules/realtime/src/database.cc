#include "motis/realtime/database.h"

#include <iostream>
#include <exception>

namespace motis {
namespace realtime {

database::database(const std::string& db, const std::string& server,
                   const std::string& user, const std::string& password)
    : conn_(false),
      db_(db),
      server_(server),
      user_(user),
      password_(password) {}

bool database::connect() {
  return conn_.connect(db_.c_str(), server_.c_str(), user_.c_str(),
                       password_.c_str());
}

delay_database::delay_database(const std::string& db, const std::string& server,
                               const std::string& user,
                               const std::string& password)
    : database(db, server, user, password),
      query_(conn_.query(
          "SELECT parsedmessage "
          "FROM ris_receiver_delay_messages "
          "WHERE received > FROM_UNIXTIME(%0q) "
          "AND received <= FROM_UNIXTIME(%1q) "
          "AND messagetype IN ('d', 'a', 'c', 's', 's', 'r', 'b', 'b') "
          "ORDER BY tout;")) {

  query_.parse();
}

std::string delay_database::get_messages(time_t t1, time_t t2) {
  // std::cout << "messages from " << t1 << " to " << t2 << std::endl;
  std::stringstream msgs;
  if (mysqlpp::StoreQueryResult res =
          query_.store(static_cast<int>(t1), static_cast<int>(t2))) {
    const unsigned int number_of_rows = res.num_rows();
    for (size_t i = 0; i < number_of_rows; ++i) {
      msgs << res[i][0] << "\n";
    }
  } else {
    std::cerr << query_.error() << "\n";
    return "";
  }
  return msgs.str();
}

}  // namespace realtime
}  // namespace motis