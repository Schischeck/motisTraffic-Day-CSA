#ifndef WEBSOCKETSERVICE_H
#define WEBSOCKETSERVICE_H

#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>
#include <set>
#include <mutex>
#include <functional>
#include "serialization/Schedule.h"
#include "RailvizProtocolV2.pb.h"

#define PROTOCOL_VERSION 2

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
    struct websocmsg {
        connection_hdl hdl;
        std::string msg;
    };
    void reply( websocmsg& );
    void reply();

    typedef std::set<connection_hdl, std::owner_less<connection_hdl>> con_list;

    int m_count;
    websocketserver m_server;
    con_list m_connections;
    std::mutex m_mutex;

    std::vector<websocmsg> m_messages;
    std::vector<std::string> DEPRECATED_m_messages;
    std::vector<StationPtr> &m_stations;
    std::string m_host, m_port;


};
}
}

#endif // WEBSOCKETSERVICE_H
