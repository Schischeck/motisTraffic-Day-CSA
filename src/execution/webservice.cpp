#include "execution/webservice.h"

#include "boost/lexical_cast.hpp"

#include "pugixml.hpp"

#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

#include "serialization/Schedule.h"
#include "Query.h"
#include "Station.h"
#include "StationGuesser.h"

using namespace net::http::server;
namespace json = rapidjson;

namespace td {
namespace execution {

webservice::webservice(Graph& graph)
    : graph_(graph),
      guesser_(new StationGuesser(graph_._sched.stations)) {
  namespace p = std::placeholders;
  enable_cors();
  route("POST", "/routing.*",
        std::bind(&webservice::query, this, p::_1, p::_2));
  route("POST", "/guesser/?(.*)",
        std::bind(&webservice::guesser, this, p::_1, p::_2));
}

webservice::~webservice() {
}

void webservice::guesser(route_request const& req, callback cb) {
  unsigned count = 10;
  if (!req.path_params[0].empty()) {
    try {
      count = boost::lexical_cast<unsigned>(req.path_params[0]);
    } catch (boost::bad_lexical_cast&) {
    }
  }

  json::StringBuffer buffer;
  json::Writer<rapidjson::StringBuffer> writer(buffer);

  writer.StartArray();
  for (td::Station const* s : guesser_->guess(req.content, count)) {
    writer.StartObject();

    writer.String("name");
    writer.String(s->name.toString().c_str());

    writer.String("latitude");
    writer.Double(s->width);

    writer.String("longitude");
    writer.Double(s->length);

    try {
      writer.String("eva_number");
      writer.Int(boost::lexical_cast<int>(s->evaNr));
    } catch (boost::bad_lexical_cast&) {
      writer.Int(-1);
    }

    writer.EndObject();
  }
  writer.EndArray();

  reply rep;
  rep.status = reply::ok;
  rep.content = buffer.GetString();
  return cb(rep);
}

void webservice::query(route_request const& req, callback cb) {
  pugi::xml_document query_doc;
  pugi::xml_parse_result result = query_doc.load(req.content.c_str());

  pugi::xml_document response;
  if (!result) {
    std::stringstream err;
    err << "Parse error (" << result.description() << ")\n";
    err << "Offset: " << result.offset << "\n";
    err << "At: [..." << req.content.substr(result.offset, 100) << "]\n";
    pugi::xml_node error = response.append_child("Error");
    error.append_attribute("reason") = err.str().c_str();
  } else {
    Query q(graph_, *guesser_.get());
    q.initFromXML(query_doc, false);
    std::cout << q.str() << "\n";
    q.execute(response);
    query_doc.child("Query").append_child("UsedOptionList");
    response.child("DataExchange").append_copy(query_doc.child("Query"));
  }

  std::stringstream out;
  pugi::xml_node decl = response.prepend_child(pugi::node_declaration);
  decl.append_attribute("version") = "1.0";
  decl.append_attribute("encoding") = "UTF-8";
  decl.append_attribute("standalone") = "no";
  response.save(out, "", pugi::format_raw);

  reply rep;
  rep.content = out.str();
  rep.status = reply::ok;

  return cb(rep);
}

}  // namespace exection
}  // namespace td