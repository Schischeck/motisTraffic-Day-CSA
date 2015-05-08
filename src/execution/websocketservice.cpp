#include "websocketservice.h"

namespace td {
namespace execution {

using websocketpp::connection_hdl;
using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
using websocketpp::lib::bind;


websocketservice::websocketservice(std::vector<StationPtr> &stations) : m_stations(stations) {
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
        m_messages.push_back(msg->get_payload());
    } else {
        //m_messages.push_back("<< " + websocketpp::utility::to_hex(msg->get_payload()));
    }
    std::cout << "MESSAGE" << std::endl;
    std::string str_req;
    std::string str_resp;
    td::Station *tmp_station;
    str_req.append(m_messages[0]);
    std::cout << "REQUEST: " << str_req << std::endl;
    m_messages.erase(m_messages.begin());
    if (str_req[0] == '0') {
        std::cout << "req was 0" << std::endl;
        str_resp.clear();
        std::cout << "str_resp_cleared" << std::endl;
        std::cout << "size: " << this->m_stations.size() << std::endl;
        for (long unsigned int i=0; i<this->m_stations.size(); i++) {

            tmp_station = m_stations[i].get();
            std::cout << "tmp_station got" << std::endl;
            if( tmp_station != NULL )
                std::cout << "station not null" << std::endl;
            str_resp.append(std::to_string(i));
            str_resp.append(",");
            str_resp.append(std::to_string(tmp_station->length));
            str_resp.append(",");
            str_resp.append(std::to_string(tmp_station->width));
            str_resp.append(";");
            std::cout << "appendet " << std::to_string(i) << "," << std::to_string(tmp_station->length) << std::endl;

            //std::cout << i << std::endl;
        }
        std::cout << "sent stations" << std::endl;
        std::cout << str_req << std::endl;
        m_server.send(hdl, str_resp, websocketpp::frame::opcode::text);
    } else if (str_req[0] == 1) {
        //
    }

    //    for (auto it : m_connections) {
    //        m_server.send(it, str, websocketpp::frame::opcode::text);
    //    }
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
