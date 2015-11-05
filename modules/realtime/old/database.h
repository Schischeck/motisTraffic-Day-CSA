#pragma once

#include <string>
#include <vector>
#include <memory>

#include "mysql++/mysql++.h"

namespace motis {
namespace realtime {

/**
* parent database class.
*/
class database {
public:
  /**
  * constructor. connects to the database using the provided information.
  *
  * \param db the database to connect to
  * \param server the server to connec to
  * \param username the username to use
  * \param password the password to use
  */
  database(const std::string& db, const std::string& server,
           const std::string& user, const std::string& password);

  /**
  * connects to the database and returns the result.
  *
  * \return true if the connection could be established successfuly
  */
  bool connect();

protected:
  mysqlpp::Connection conn_;

private:
  std::string db_;
  std::string server_;
  std::string user_;
  std::string password_;
};

/**
* the delay_database class is responsible for providing access to the
* delay messages located in the delay messages my_s_q_l database.
*/
class delay_database : public database {
public:
  /**
  * constructor. connects to the database using the provided information.
  *
  * \param db the database to connect to
  * \param server the server to connec to
  * \param username the username to use
  * \param password the password to use
  */
  delay_database(const std::string& db, const std::string& server,
                 const std::string& user, const std::string& password);

  /**
  * get all delay messages that were created between t2 and t2.
  *
  * \param t1 start of time interval
  * \param t2 end of time interval
  * \return all messages in the time interval
  */
  std::string get_messages(time_t t1, time_t t2);

private:
  mysqlpp::Query query_;
};

}  // namespace realtime
}  // namespace motis
