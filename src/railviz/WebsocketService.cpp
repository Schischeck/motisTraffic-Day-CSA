#include "WebsocketService.h"
#include "../Logging.h"
#include "../Nodes.h"

namespace td {
namespace railviz {

using websocketpp::connection_hdl;
using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
using websocketpp::lib::bind;


WebsocketService::WebsocketService(td::Schedule &schedule,
                                   std::string &host, std::string &port) : m_schedule(schedule) {
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
    if (msg->get_opcode() == websocketpp::frame::opcode::binary) {
        websocmsg message;
        message.hdl = hdl;
        message.msg = msg->get_payload();
        //m_messages.push_back(message);
        //this->reply();
        this->reply( message );
    }
    return;
}

void WebsocketService::reply( websocmsg& message ) {
    protocol::Request request;
    protocol::Response response;
    response.set_protocol_version(PROTOCOL_VERSION);


    if( request.ParseFromString( message.msg ) )
    {
        if( request.protocol_version() != PROTOCOL_VERSION )
        {
            response.set_type( protocol::Response::ERROR );
            response.set_msg("The Protocol-Version is not supportet");
        } else
        {

            if( request.type() == protocol::Request::ALL_STATIONS )
            {
                response.set_type( protocol::Response::ALL_STATIONS );
                for (long unsigned int i=0; i < this->m_schedule.stations.size(); i++) {
                    protocol::Station* station = response.add_stations();
                    station->set_id(i);
                    station->set_latitude(this->m_schedule.stations[i].get()->width);
                    station->set_longitude(this->m_schedule.stations[i].get()->length);
                }
            }
            else if( request.type() == protocol::Request::DETAILED_STATION )
            {
                response.set_type( protocol::Response::DETAILED_STATION );
                for (unsigned int i=0; i < request.stations_size(); i++) {
                    int id = request.stations(i).id();
                    protocol::Station* station = response.add_stations();
                    station->set_id(id);
                    station->set_name(this->m_schedule.stations[id].get()->name);
                }
            }
            else if ( request.type() == protocol::Request::ALL_TRAINS )
            {
                response.set_type( protocol::Response::ALL_TRAINS );
                for (unsigned int i=0; i < this->m_schedule.stationNodes.size(); i++) {
                    td::StationNode* station_node = m_schedule.stationNodes.at(i).get();
                    std::vector<td::Node*> route_nodes = station_node->getRouteNodes();
                    //TODO JUST DUMMY TO FETCH ALL clasz == 0 (ICE and co.)
                    for (td::Node* node : route_nodes) {
                        if (node->isRouteNode()) {
                            //TODO
                        }
                    }
                }
            }
            else
            {
                response.set_type( protocol::Response::ERROR );
                response.set_msg("Invalid Request-Type");
            }
        }
    } else
    {
        response.set_type( protocol::Response::ERROR );
        response.set_msg("Failed to parse Request");
    }

    m_server.send(message.hdl, response.SerializeAsString(), websocketpp::frame::opcode::binary);
}

//void WebsocketService::reply() {
//    protocol::Request request;
//    protocol::Response response;
//    response.set_protocol_version(PROTOCOL_VERSION);

//    for (websocmsg message: m_messages) {

//        if( request.ParseFromString( message.msg ) )
//        {
//            if( request.protocol_version() != PROTOCOL_VERSION )
//            {
//                response.set_type( protocol::Response::ERROR );
//                response.set_msg("The Protocol-Version is not supportet");
//            } else
//            {

//                if( request.type() == protocol::Request::ALL_STATIONS )
//                {
//                    response.set_type( protocol::Response::ALL_STATIONS );
//                    for (long unsigned int i=0; i<this->m_stations.size(); i++) {
//                        protocol::Station* station = response.add_stations();
//                        station->set_id(i);
//                        station->set_latitude(m_stations[i].get()->width);
//                        station->set_longitude(m_stations[i].get()->length);
//                    }
//                }
//                else if( request.type() == protocol::Request::DETAILED_STATION )
//                {
//                    response.set_type( protocol::Response::DETAILED_STATION );
//                    for (long unsigned int i=0; i < request.stations_size(); i++) {
//                        int id = request.stations(i).id();
//                        protocol::Station* station = response.add_stations();
//                        station->set_id(id);
//                        station->set_name(m_stations[id].get()->name);
//                    }
//                }
//                else
//                {
//                    response.set_type( protocol::Response::ERROR );
//                    response.set_msg("Invalid Request-Type");
//                }
//            }
//        } else
//        {
//            response.set_type( protocol::Response::ERROR );
//            response.set_msg("Failed to parse Request");
//        }

//        m_server.send(message.hdl, response.SerializeAsString(), websocketpp::frame::opcode::binary);
//    }
//    m_messages.clear();
//    return;
//}

void WebsocketService::run() {
    m_server.listen(m_host, m_port);
    m_server.start_accept();
    LOG(td::logging::info) << "Websocketservice serving requests..." << std::endl;
    m_server.run();
}

void *WebsocketService::runHelper(void *classRef) {
    ((WebsocketService *) classRef)->run();
    return NULL;
}

}
}
