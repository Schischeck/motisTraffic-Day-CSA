#include <iostream>

#include "boost/asio/io_service.hpp"
#include "boost/date_time/posix_time/posix_time.hpp"
#include "boost/asio/deadline_timer.hpp"

#include "net/http/server/server.hpp"
#include "net/http/server/query_router.hpp"
#include "net/http/server/shutdown_handler.hpp"
#include "net/tcp.h"

#include "conf/options_parser.h"

#include "Logging.h"
#include "Label.h"
#include "Graph.h"
#include "serialization/Schedule.h"
#include "serialization/Print.h"
#include "execution/listener_settings.h"
#include "execution/dataset_settings.h"
#include "execution/callback_settings.h"
#include "execution/webservice.h"

#include "railviz/WebsocketService.h"
#include "railviz/WebsocketServiceSettings.h"
#include <thread>

using namespace td::execution;
using namespace net::http::server;
using boost::system::error_code;

void management_callback(
        boost::asio::io_service& ios,
        callback_settings const& callback_opt) {
    if (!callback_opt.enabled) {
        return;
    }

    std::make_shared<net::tcp>(
                ios, callback_opt.host, callback_opt.port,
                boost::posix_time::seconds(30))->connect(
                [&callback_opt](std::shared_ptr<net::tcp> con, error_code ec) {
        if (ec) {
            std::cerr << "ERROR: cannot connect to callback port: "
                      << ec.message() << "\n";
        } else {
            std::string msg = callback_opt.id + "\n";
            msg += "\0";

            boost::asio::async_write(con->socket_, boost::asio::buffer(msg),
                                     [](error_code ec, std::size_t) {
                if (ec) {
                    std::cerr << "ERROR: cannot send message to callback port: "
                              << ec.message() << "\n";
                } else {
                    std::cout << "CALLBACK SUCCESSFUL!" << std::endl;
                }
            });
        }
    });
}

int main(int argc, char* argv[]) {
    listener_settings listener_opt(false, "127.0.0.1", "8080");

    dataset_settings dataset_opt("data/test");
    callback_settings callback_opt(false, "127.0.0.1", "", "");
    td::railviz::WebsocketServiceSettings web_soc_opt(false, "127.0.0.1", "9002");
    conf::options_parser parser({ &listener_opt, &web_soc_opt, &dataset_opt, &callback_opt });
    parser.read_command_line_args(argc, argv);

    if (parser.help()) {
        std::cout << "\n\tTime Dependent Routing\n\n";
        parser.print_help(std::cout);
        return 0;
    } else if (parser.version()) {
        std::cout << "Time Dependent Routing\n";
        return 0;
    }

    parser.read_configuration_file();

    std::cout << "\n\tTime Dependent Routing\n\n";
    parser.print_unrecognized(std::cout);
    parser.print_used(std::cout);

    td::Schedule sched(dataset_opt.dataset);
    td::MemoryManager<td::Label> labelStore(MAX_LABELS_WITH_MARGIN);
    td::Graph graph(sched, labelStore);

    boost::asio::io_service ios;


    //define threads
    std::thread web_soc_thread;
    std::thread web_thread;


    webservice ws(graph);
    server http_server(ios, listener_opt.host, listener_opt.port,
                       std::reference_wrapper<webservice>(ws));
    management_callback(ios, callback_opt);

    shutdown_handler<server> server_shutdown_handler(ios, http_server);

    td::railviz::WebsocketService websocketsrv(sched.stations, web_soc_opt.web_soc_host, web_soc_opt.web_soc_port);
    if (web_soc_opt.web_soc_enabled) {
        web_soc_thread = std::thread(std::bind(&td::railviz::WebsocketService::run, &websocketsrv));
    } 

    LOG(td::logging::info) << "Starting..." << std::endl;

    if (listener_opt.enabled) {
        LOG(td::logging::info) << "Webservice serving requests..." << std::endl;
        //web_thread == std::thread(std::bind(&boost::asio::io_service::run, &ios));
        ios.run();
    }

    if (web_soc_thread.joinable()) {
        web_soc_thread.join();  
    }

    if (web_thread.joinable()) {
        web_thread.join();
    }

    std::cout << "quit\n";
}
