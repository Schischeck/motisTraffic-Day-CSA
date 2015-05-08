#include <set>
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>

namespace td {
namespace execution {
typedef websocketpp::server<websocketpp::config::asio> websocketserver;

using websocketpp::connection_hdl;
using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
using websocketpp::lib::bind;

class websocketservice {
public:
    websocketservice() {
        m_server.init_asio();

        m_server.set_open_handler(bind(&websocketservice::on_open, this, ::_1));
        m_server.set_close_handler(bind(&websocketservice::on_close, this, ::_1));
        m_server.set_message_handler(bind(&websocketservice::on_message, this, ::_1, ::_2));
    }

    void on_open(connection_hdl hdl) {
        m_connections.insert(hdl);
    }

    void on_close(connection_hdl hdl) {
        m_connections.erase(hdl);
    }

    void on_message(connection_hdl hdl, websocketserver::message_ptr msg) {
        for (auto it : m_connections) {
            m_server.send(it,msg);
        }
    }

    void run(uint16_t port) {
        m_server.listen(port);
        m_server.start_accept();
        m_server.run();
    }

private:
    typedef std::set<connection_hdl, std::owner_less<connection_hdl>> con_list;

    websocketserver m_server;
    con_list m_connections;
};
}
}

//#include "websocketservice.h"


//namespace td {
//namespace execution {


//websocketservice::websocketservice() :  m_count(0) {
//    m_server.init_asio();

//    m_server.set_open_handler(bind(&websocketservice::on_open,this,_1));
//    m_server.set_close_handler(bind(&websocketservice::on_close,this,_1));
//}

//void websocketservice::on_open(connection_hdl hdl) {
//    std::lock_guard<std::mutex> lock(m_mutex);
//    m_connections.insert(hdl);
//}

//void websocketservice::on_close(connection_hdl hdl) {
//    std::lock_guard<std::mutex> lock(m_mutex);
//    m_connections.erase(hdl);
//}

//void websocketservice::count() {
//    while (true) {
//        sleep(1);
//        m_count++;

//        std::stringstream ss;
//        ss << m_count;

//        std::lock_guard<std::mutex> lock(m_mutex);
//        for (auto it : m_connections) {
//            m_server.send(it,ss.str(),websocketpp::frame::opcode::text);
//        }
//    }
//}

//void websocketservice::run(uint16_t port) {
//    m_server.listen(port);
//    m_server.start_accept();
//    m_server.run();
//}
//}   // namespace execution
//}   // namespace td
