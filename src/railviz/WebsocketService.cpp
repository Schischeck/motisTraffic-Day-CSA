#include "WebsocketService.h"

namespace td {
namespace railviz {

using websocketpp::connection_hdl;
using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
using websocketpp::lib::bind;


WebsocketService::WebsocketService(std::vector<StationPtr> &stations,
                                   std::string &host, std::string &port) : m_stations(stations) {
    m_server.init_asio();

    m_server.clear_access_channels(websocketpp::log::alevel::all);
    m_server.set_open_handler(bind(&WebsocketService::on_open, this, ::_1));
    m_server.set_close_handler(bind(&WebsocketService::on_close, this, ::_1));
    m_server.set_message_handler(bind(&WebsocketService::on_message, this, ::_1, ::_2));
    m_host = host;
    m_port = port;
}

WebsocketService::~WebsocketService() {
}

void WebsocketService::on_open(connection_hdl hdl) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_connections.insert(hdl);
}

void WebsocketService::on_close(connection_hdl hdl) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_connections.erase(hdl);
}

void WebsocketService::on_message(connection_hdl hdl, websocketserver::message_ptr msg) {
    if (msg->get_opcode() == websocketpp::frame::opcode::text) {
        websocmsg message;
        message.hdl = hdl;
        message.msg = msg->get_payload();
        m_messages.push_back(message);
        this->reply();

        //m_messages.push_back(message);
        //return;

        //        DEPRECATED_m_messages.push_back(msg->get_payload());
        //        std::string str_req;
        //        std::string str_resp;
        //        td::Station *tmp_station;
        //        str_req.append(DEPRECATED_m_messages[0]);
        //        DEPRECATED_m_messages.erase(DEPRECATED_m_messages.begin());
        //        if (str_req[0] == '0') {
        //            str_resp.clear();
        //            for (long unsigned int i=0; i<this->m_stations.size(); i++) {

        //                tmp_station = m_stations[i].get();
        //                str_resp.append(std::to_string(i));
        //                str_resp.append(",");
        //                str_resp.append(std::to_string(tmp_station->width));
        //                str_resp.append(",");
        //                str_resp.append(std::to_string(tmp_station->length));
        //                str_resp.append(";");
        //            }
        //            m_server.send(hdl, str_resp, websocketpp::frame::opcode::text);
        //        } else if (str_req[0] == '1' && str_req[1] == ',') {
        //            long unsigned int i = 0;
        //            std::stringstream ss;
        //            ss<<str_req.substr(2);
        //            ss>>i;
        //            m_server.send(hdl, m_stations[i].get()->name, websocketpp::frame::opcode::text);
        //        }
        //        return;
    }
    return;
}

void WebsocketService::reply() {
    RequestPovUpdate req_pov_update;
    RequestAllStations req_all_stations;
    RequestGetStationsInfo req_get_stations_info;
    RequestAllTrains req_all_trains;
    for (websocmsg message: m_messages) {
        if (req_pov_update.ParseFromString(message.msg)) {
           //TODO
        } else if (req_all_stations.ParseFromString(message.msg)) {
            if (req_all_stations.protoversion() != PROTOCOL_VERSION) continue;
            ResponseAllStatinos resp_all_stations;
            resp_all_stations.set_protoversion(PROTOCOL_VERSION);
            for (long unsigned int i=0; i<this->m_stations.size(); i++) {
                ResponseAllStatinos_Station* station = resp_all_stations.add_station();
                station->set_stationid(i);
                station->set_station_latitude(m_stations[i].get()->width);
                station->set_station_longitude(m_stations[i].get()->length);
            }
            m_server.send(message.hdl, resp_all_stations.SerializeAsString(), websocketpp::frame::opcode::text);
        }
    }
    m_messages.clear();
    return;
}

void WebsocketService::run() {
    m_server.listen(m_host, m_port);
    m_server.start_accept();
    m_server.run();
}

void *WebsocketService::runHelper(void *classRef) {
    ((WebsocketService *) classRef)->run();
    return NULL;
}

}
}
