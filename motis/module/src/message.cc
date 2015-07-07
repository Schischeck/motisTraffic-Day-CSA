#include "motis/module/message.h"

#include "flatbuffers/idl.h"
#include "flatbuffers/util.h"

#include "motis/protocol/resources.h"

namespace motis {
namespace module {

std::unique_ptr<flatbuffers::Parser> message::parser =
    std::unique_ptr<flatbuffers::Parser>(new flatbuffers::Parser());

void message::init_parser() {
  int message_symbol_index = -1;
  for (int i = 0; i < number_of_symbols; ++i) {
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
  bool parse_ok = parser->Parse(json.c_str());
  if (!parse_ok) {
    throw std::runtime_error("error while parsing JSON");
  }

  buf_ = parser->builder_.GetBufferPointer();
  msg_ = motis::GetMessage(buf_);
  mem_ = parser->builder_.ReleaseBufferPointer();

  parser->builder_.Clear();
}

std::string message::to_json() const {
  auto opt = flatbuffers::GeneratorOptions();
  opt.strict_json = true;

  std::string json;
  flatbuffers::GenerateText(*parser, buf_, opt, &json);

  return json;
}

}  // namespace module
}  // namespace motis
