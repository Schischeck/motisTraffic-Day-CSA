#ifndef WEBSOCKETSERVICE_H
#define WEBSOCKETSERVICE_H

#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>

using websocketpp::connection_hdl;

namespace td {
namespace execution {

typedef websocketpp::server<websocketpp::config::asio> websocketserver;

class websocketservice {
public:
    websocketservice();
    ~websocketservice();

    void on_open(connection_hdl hdl);
    void on_close(connection_hdl hdl);
    void on_message(connection_hdl hdl, websocketserver::message_ptr msg);
    void run(uint16_t port);
private:
    typedef std::set<connection_hdl, std::owner_less<connection_hdl>> con_list;

    websocketserver m_server;
    con_list m_connections;
};
}
}

//#include <functional>
//#include <mutex>
//#include <set>
//#include <thread>

//#include <websocketpp/config/asio_no_tls.hpp>
//#include <websocketpp/server.hpp>

//typedef websocketpp::server<websocketpp::config::asio> websocketsrv;

//using websocketpp::connection_hdl;

//namespace td {

//namespace execution {

//class websocketservice {
//public:
//    websocketservice();
//    ~websocketservice();

//    void on_open(websocketpp::connection_hdl hd1);
//    void on_close(websocketpp::connection_hdl hd1);
//    void count();
//    void run(uint16_t port);
//private:
//    typedef std::set<websocketpp::connection_hdl, std::owner_less<websocketpp::connection_hdl>> con_list;

//    int m_count;
//    websocketsrv m_server;
////    con_list m_connections;
//    std::mutex m_mutex;
//};

//}   // namespace execution
//}   // namespace td

#endif // WEBSOCKETSERVICE_H
