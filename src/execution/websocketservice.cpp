#include "websocketservice.h"


namespace td {
namespace execution {


class websocketservice {
public:
    websocketservice() :  m_count(0) {
        m_server.init_asio();

        m_server.set_open_handler(bind(&websocketservice::on_open,this,_1));
        m_server.set_close_handler(bind(&websocketservice::on_close,this,_1));
    }

    void on_open(connection_hdl hdl) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_connections.insert(hdl);
    }

    void on_close(connection_hdl hdl) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_connections.erase(hdl);
    }

    void count() {
        while (true) {
            sleep(1);
            m_count++;

            std::stringstream ss;
            ss << m_count;

            std::lock_guard<std::mutex> lock(m_mutex);
            for (auto it : m_connections) {
                m_server.send(it,ss.str(),websocketpp::frame::opcode::text);
            }
        }
    }

    void run(uint16_t port) {
        m_server.listen(port);
        m_server.start_accept();
        m_server.run();
    }
};
}   // namespace execution
}   // namespace td
