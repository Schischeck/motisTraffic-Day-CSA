#ifndef TD_EXECUTION_WEBSERVICE_
#define TD_EXECUTION_WEBSERVICE_

#include <memory>

#include "net/http/server/query_router.hpp"

namespace td {

class Graph;
class StationGuesser;

namespace execution {

class webservice : public net::http::server::query_router {
public:
  webservice(Graph& graph);
  ~webservice();

  void query(net::http::server::route_request const& req,
             net::http::server::callback cb);

  void guesser(net::http::server::route_request const& req,
               net::http::server::callback cb);


private:
  Graph& graph_;
  std::unique_ptr<StationGuesser> guesser_;
};

}  // namespace execution
}  // namespace td

#endif  // TD_EXECUTION_WEBSERVICE_
