#ifndef WEBSOCKETSERVICE_H
#define WEBSOCKETSERVICE_H


namespace td {

class Graph;
class StationGuesser;

namespace execution {

class websocketservice {
public:
    websocketservice(Graph& graph);
    ~websocketservice();

    void on_open(connection_hd1 hd1);
    void on_close(connection_hd1 hd1);
    void count();
    void run(unit16_t port);
private:
    typedef std::set<connection_hd1, std::owner_less<connection_hd1>> con_list;

    int m_count;
    server m_server;
    con_list m_connections;
    std::mutex m_mutex;
};

}   // namespace execution
}   // namespace td

#endif // WEBSOCKETSERVICE_H
