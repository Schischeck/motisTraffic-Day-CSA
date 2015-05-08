#include "websocketservice.h"

namespace td {
namespace execution {

using websocketpp::connection_hdl;
using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
using websocketpp::lib::bind;


websocketservice::websocketservice() {
    m_server.init_asio();

    m_server.set_open_handler(bind(&websocketservice::on_open, this, ::_1));
    m_server.set_close_handler(bind(&websocketservice::on_close, this, ::_1));
    m_server.set_message_handler(bind(&websocketservice::on_message, this, ::_1, ::_2));
}

websocketservice::~websocketservice() {
}

void websocketservice::on_open(connection_hdl hdl) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_connections.insert(hdl);
}

void websocketservice::on_close(connection_hdl hdl) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_connections.erase(hdl);
}

void websocketservice::on_message(connection_hdl hdl, websocketserver::message_ptr msg) {
    if (msg->get_opcode() == websocketpp::frame::opcode::text) {
        m_messages.push_back("<< " + msg->get_payload());
    } else {
        m_messages.push_back("<< " + websocketpp::utility::to_hex(msg->get_payload()));
    }
    std::string str;
    str.append(m_messages[0]);
    m_messages.erase(m_messages.begin());
    str.append("jsdkfdsjflkd");
    for (auto it : m_connections) {
        m_server.send(it, str, websocketpp::frame::opcode::text);
    }
}


void websocketservice::count() {
    while (1) {
        sleep(1);
        m_count++;

        std::stringstream ss;
        ss << m_count;

        std::lock_guard<std::mutex> lock(m_mutex);
        for (auto it : m_connections) {
            m_server.send(it,ss.str() + "10", websocketpp::frame::opcode::text);
        }
    }
}

void websocketservice::run(uint16_t port) {
    m_server.listen( boost::asio::ip::tcp::v4(), port);
    m_server.start_accept();
    m_server.run();
}

}
}
