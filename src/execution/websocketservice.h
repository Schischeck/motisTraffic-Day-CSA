#ifndef WEBSOCKETSERVICE_H
#define WEBSOCKETSERVICE_H

#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>
#include <set>
#include <mutex>
#include <functional>
#include "serialization/Schedule.h"

using websocketpp::connection_hdl;

namespace td {
namespace execution {

typedef websocketpp::server<websocketpp::config::asio> websocketserver;

class websocketservice {
public:
    websocketservice(std::vector<StationPtr> &stations);
    ~websocketservice();

    void on_open(connection_hdl hdl);
    void on_close(connection_hdl hdl);
    void on_message(connection_hdl hdl, websocketserver::message_ptr msg);
    void run(uint16_t port);
private:
    typedef std::set<connection_hdl, std::owner_less<connection_hdl>> con_list;

    int m_count;
    websocketserver m_server;
    con_list m_connections;
    std::mutex m_mutex;

    std::vector<std::string> m_messages;
//    td::Schedule &m_schedule;
    std::vector<StationPtr> &m_stations;
};
}
}

#endif // WEBSOCKETSERVICE_H
