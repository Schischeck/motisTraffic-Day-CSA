#include "motis/bikesharing/nextbike_parser.h"

#include "pugixml.hpp"

#include "parser/arg_parser.h"

using namespace pugi;
using namespace parser;

namespace motis {
namespace bikesharing {

std::time_t nextbike_filename_to_timestamp(std::string const& filename) {
  auto dash_pos = filename.rfind("-");
  auto dot_pos = filename.rfind(".");
  if (dash_pos == std::string::npos || dot_pos == std::string::npos ||
      dash_pos > dot_pos) {
    throw std::runtime_error("unexpected nextbike filename");
  }

  return std::stoul(filename.substr(dash_pos+1, dot_pos-dash_pos-1));
}

std::vector<terminal_snapshot> nextbike_parse_xml(parser::buffer&& buffer) {
  std::vector<terminal_snapshot> result;

  xml_document d;
  d.load_buffer_inplace(reinterpret_cast<void*>(buffer.data()), buffer.size());

  constexpr char const* query = "/markers/country[@country='DE']/city/place";
  for (auto const& xnode : d.select_nodes(query)) {
    auto const& node = xnode.node();

    terminal_snapshot terminal;
    terminal.uid = node.attribute("uid").as_int();
    terminal.lat = node.attribute("lat").as_double();
    terminal.lng = node.attribute("lng").as_double();
    terminal.name = node.attribute("name").value();
    terminal.available_bikes = parse<int>(node.attribute("bikes").value(), 0);
    result.push_back(terminal);
  }

  return result;
}

}  // namespace bikesharing
}  // namespace motis
