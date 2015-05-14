#include "railviz/WebsocketServiceSettings.h"
#include <ostream>
#include "boost/program_options.hpp"

#define WEB_SOC_ENABLED "websocket.enabled"
#define WEB_SOC_HOST "websocket.host"
#define WEB_SOC_PORT "websocket.port"

namespace po = boost::program_options;

namespace td {
namespace railviz {

WebsocketServiceSettings::WebsocketServiceSettings(bool default_enable,
                                                       std::string default_host,
                                                       std::string default_port)
    : web_soc_enabled(default_enable),
      web_soc_host(default_host),
      web_soc_port(default_port) {
}

po::options_description WebsocketServiceSettings::desc() {
    po::options_description desc("Websocket Service Options");
    desc.add_options()
            (WEB_SOC_ENABLED,
             po::value<bool>(&web_soc_enabled)->default_value(web_soc_enabled),
             "enable websocket service (e.g. true or false)")
            (WEB_SOC_HOST,
             po::value<std::string>(&web_soc_host)->default_value(web_soc_host),
             "host to listen on (e.g. 0.0.0.0 or 127.0.0.1)")
            (WEB_SOC_PORT,
             po::value<std::string>(&web_soc_port)->default_value(web_soc_port),
             "port to listen on (e.g. 9002)");
    return desc;
}

void WebsocketServiceSettings::print(std::ostream &out) const {
    out << "  " << WEB_SOC_ENABLED << ": " << std::boolalpha << web_soc_enabled << "\n"
        << "  " << WEB_SOC_HOST << ": " << web_soc_host << "\n"
        << "  " << WEB_SOC_PORT << ": " << web_soc_port;
}

}  // namespace railviz
}  // namespace td
