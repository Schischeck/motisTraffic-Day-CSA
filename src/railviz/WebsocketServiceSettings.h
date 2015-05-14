#ifndef WEBSOCKET_SERVICE_SETTINGS
#define WEBSOCKET_SERVICE_SETTINGS

#include <string>
#include "conf/configuration.h"

namespace td {
namespace railviz {

class WebsocketServiceSettings : public conf::configuration {
public:
    WebsocketServiceSettings(bool default_enabled,
                               std::string default_host,
                               std::string default_port);

    virtual ~WebsocketServiceSettings() {}

    virtual boost::program_options::options_description desc() override;

    virtual void print(std::ostream& out) const override;

    bool web_soc_enabled;
    std::string web_soc_host, web_soc_port;
};

}  // namespace railviz
}  // namespace td

#endif // WEBSOCKET_SERVICE_SETTINGS
