#include "motis/module/message.h"

#include <stdexcept>

#include "flatbuffers/idl.h"
#include "flatbuffers/util.h"

#include "motis/protocol/resources.h"

#include "motis/module/error.h"

#undef GetMessage

namespace motis {
namespace module {

std::unique_ptr<flatbuffers::Parser> message::parser =
    std::unique_ptr<flatbuffers::Parser>(new flatbuffers::Parser());

void message::init_parser() {
  int message_symbol_index = -1;
  for (unsigned i = 0; i < number_of_symbols; ++i) {
    if (strcmp(filenames[i], "Message.fbs") == 0) {
      message_symbol_index = i;
    } else if (!parser->Parse((const char*)symbols[i], nullptr, filenames[i])) {
      printf("error: %s\n", parser->error_.c_str());
      throw std::runtime_error("flatbuffer protocol definitions parser error");
    }
  }
  if (message_symbol_index == -1 ||
      !parser->Parse((const char*)symbols[message_symbol_index])) {
    printf("error: %s\n", parser->error_.c_str());
    throw std::runtime_error("flatbuffer protocol definitions parser error");
  }
}

message::message(std::string const& json) {
  if (!parser->root_struct_def_) {
    init_parser();
  }

  bool parse_ok = parser->Parse(json.c_str());
  if (!parse_ok) {
    throw boost::system::system_error(error::unable_to_parse_msg);
  }

  flatbuffers::Verifier verifier(parser->builder_.GetBufferPointer(),
                                 parser->builder_.GetSize());
  if (!VerifyMessageBuffer(verifier)) {
    throw boost::system::system_error(error::malformed_msg);
  }

  buf_ = parser->builder_.GetBufferPointer();
  msg_ = motis::GetMutableMessage(buf_);
  mem_ = parser->builder_.ReleaseBufferPointer();

  parser->builder_.Clear();
}

std::string message::to_json() const {
  if (!parser->root_struct_def_) {
    init_parser();
  }

  auto opt = flatbuffers::GeneratorOptions();
  opt.strict_json = true;

  std::string json;
  flatbuffers::GenerateText(*parser, buf_, opt, &json);

  return json;
}

}  // namespace module
}  // namespace motis
