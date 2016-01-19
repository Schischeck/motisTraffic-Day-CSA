#include "motis/bikesharing/nextbike_parser.h"

#include "pugixml.hpp"

#include "parser/arg_parser.h"

using namespace pugi;
using namespace parser;

namespace motis {
namespace bikesharing {

std::vector<parsed_bikesharing_terminal> parse_nextbike_xml(
    parser::buffer&& buffer) {
  std::vector<parsed_bikesharing_terminal> result;

  xml_document d;
  d.load_buffer_inplace(reinterpret_cast<void*>(buffer.data()), buffer.size());

  for (auto const& xpath_node : d.select_nodes("//place")) {
    auto const& node = xpath_node.node();

    parsed_bikesharing_terminal terminal;
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
