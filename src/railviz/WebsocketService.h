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
namespace railviz {

typedef websocketpp::server<websocketpp::config::asio> websocketserver;

class WebsocketService {
public:
    WebsocketService(std::vector<StationPtr> &stations, std::string &host, std::string &port);
    ~WebsocketService();

    void on_open(connection_hdl hdl);
    void on_close(connection_hdl hdl);
    void on_message(connection_hdl hdl, websocketserver::message_ptr msg);
    void run();
    static void *runHelper(void *classRef);
private:
    typedef std::set<connection_hdl, std::owner_less<connection_hdl>> con_list;

    int m_count;
    websocketserver m_server;
    con_list m_connections;
    std::mutex m_mutex;

    std::vector<std::string> m_messages;
//    td::Schedule &m_schedule;
    std::vector<StationPtr> &m_stations;
    std::string m_host, m_port;
};
}
}

#endif // WEBSOCKETSERVICE_H
