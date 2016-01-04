#include "motis/module/message.h"

#include <cstring>
#include <stdexcept>

#include "flatbuffers/idl.h"
#include "flatbuffers/util.h"

#include "motis/core/common/logging.h"
#include "motis/module/error.h"
#include "motis/protocol/resources.h"

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
    LOG(motis::logging::error) << "parse error: " << parser->error_;
    throw boost::system::system_error(error::unable_to_parse_msg);
  }

  flatbuffers::Verifier verifier(parser->builder_.GetBufferPointer(),
                                 parser->builder_.GetSize());
  if (!VerifyMessageBuffer(verifier)) {
    throw boost::system::system_error(error::malformed_msg);
  }

  len_ = parser->builder_.GetSize();
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

msg_ptr make_msg(std::string const& json) {
  return std::make_shared<message>(json);
}

msg_ptr make_msg(MessageCreator& builder) {
  auto len = builder.GetSize();
  auto buf = builder.GetBufferPointer();
  auto msg = GetMutableMessage(buf);
  auto mem = builder.ReleaseBufferPointer();
  builder.Clear();
  return std::make_shared<message>(len, std::move(mem), msg, buf);
}

msg_ptr make_msg(void* buf, size_t len) {
  flatbuffers::unique_ptr_t mem(static_cast<uint8_t*>(operator new(len)),
                                std::default_delete<uint8_t>());
  std::memcpy(mem.get(), buf, len);

  flatbuffers::Verifier verifier(mem.get(), len);
  if (!VerifyMessageBuffer(verifier)) {
    throw boost::system::system_error(error::malformed_msg);
  }

  void* ptr = mem.get();
  return std::make_shared<message>(len, std::move(mem), GetMutableMessage(ptr),
                                   buf);
}

}  // namespace module
}  // namespace motis
