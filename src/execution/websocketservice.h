#ifndef WEBSOCKETSERVICE_H
#define WEBSOCKETSERVICE_H

#include <functional>
#include <mutex>
#include <set>
#include <thread>

#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>

typedef websocketpp::server<websocketpp::config::asio> websocketsrv;

using websocketpp::connection_hdl;

namespace td {

//class Graph;
//class StationGuesser;

namespace execution {

class websocketservice {
public:
    websocketservice();
    ~websocketservice();

    void on_open(websocketpp::connection_hdl hd1);
    void on_close(websocketpp::connection_hdl hd1);
    void count();
    void run(uint16_t port);
private:
    typedef std::set<websocketpp::connection_hdl, std::owner_less<websocketpp::connection_hdl>> con_list;

    int m_count;
    websocketsrv m_server;
    con_list m_connections;
    std::mutex m_mutex;
};

}   // namespace execution
}   // namespace td

#endif // WEBSOCKETSERVICE_H
