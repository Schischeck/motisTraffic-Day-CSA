#include "websocketservice.h"

namespace td {
namespace execution {

using websocketpp::connection_hdl;
using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
using websocketpp::lib::bind;


websocketservice::websocketservice(std::vector<StationPtr> &stations) : m_stations(stations) {
    m_server.init_asio();

    m_server.clear_access_channels(websocketpp::log::alevel::all);
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
        m_messages.push_back(msg->get_payload());
    } else {
        //m_messages.push_back("<< " + websocketpp::utility::to_hex(msg->get_payload()));
    }
    std::string str_req;
    std::string str_resp;
    td::Station *tmp_station;
    str_req.append(m_messages[0]);
    m_messages.erase(m_messages.begin());
    if (str_req[0] == '0') {
        str_resp.clear();
        for (long unsigned int i=0; i<this->m_stations.size(); i++) {

            tmp_station = m_stations[i].get();
            str_resp.append(std::to_string(i));
            str_resp.append(",");
            str_resp.append(std::to_string(tmp_station->width));
            str_resp.append(",");
            str_resp.append(std::to_string(tmp_station->length));
            str_resp.append(";");
        }
        m_server.send(hdl, str_resp, websocketpp::frame::opcode::text);
    } else if (str_req[0] == '1' && str_req[1] == ',') {
        long unsigned int i = 0;
        std::stringstream ss;
        ss<<str_req.substr(2);
        ss>>i;
        m_server.send(hdl, m_stations[i].get()->name, websocketpp::frame::opcode::text);
    }

    //    for (auto it : m_connections) {
    //        m_server.send(it, str, websocketpp::frame::opcode::text);
    //    }
}

void websocketservice::run(uint16_t port) {
    m_server.listen( boost::asio::ip::tcp::v4(), port);
    m_server.start_accept();
    m_server.run();
}

}
}
