#include "motis/guesser/guesser.h"

#include <iostream>

#include "boost/program_options.hpp"

#include "motis/module/api.h"

#include "motis/protocol/Message_generated.h"

using namespace flatbuffers;
using namespace motis::module;
namespace po = boost::program_options;

namespace motis {
namespace guesser {

po::options_description guesser::desc() {
  po::options_description desc("Guesser Module");
  return desc;
}

void guesser::print(std::ostream& out) const {}

msg_ptr guesser::on_msg(msg_ptr const& msg, sid) {
  FlatBufferBuilder fbb;

  std::vector<Offset<String>> guesses;
  guesses.emplace_back(fbb.CreateString("a"));
  guesses.emplace_back(fbb.CreateString("b"));
  guesses.emplace_back(fbb.CreateString("c"));
  auto vec = fbb.CreateVector(std::move(guesses));
  auto mloc = CreateStationGuesserResponse(fbb, vec);

  auto mmloc = motis::CreateMessage(
      fbb, motis::MsgContent_StationGuesserResponse, mloc.Union());
  fbb.Finish(mmloc);

  return make_msg(fbb);
}

MOTIS_MODULE_DEF_MODULE(guesser)

}  // namespace guesser
}  // namespace motis
