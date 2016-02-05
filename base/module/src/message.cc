#include "motis/module/message.h"

#include <cstring>
#include <stdexcept>

#include "flatbuffers/idl.h"
#include "flatbuffers/util.h"

#include "motis/core/common/logging.h"
#include "motis/core/common/util.h"
#include "motis/module/error.h"
#include "motis/protocol/resources.h"

#undef GetMessage

using namespace flatbuffers;

namespace motis {
namespace module {

std::unique_ptr<Parser> init_parser() {
  auto parser = make_unique<Parser>();
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
  return parser;
}
static std::unique_ptr<Parser> parser = init_parser();

std::string message::to_json() const {
  auto opt = flatbuffers::GeneratorOptions();
  opt.strict_json = true;

  std::string json;
  flatbuffers::GenerateText(*parser, data(), opt, &json);

  return json;
}

msg_ptr make_msg(std::string const& json) {
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
  auto size = parser->builder_.GetSize();
  auto buffer = parser->builder_.ReleaseBufferPointer();

  parser->builder_.Clear();
  return std::make_shared<message>(size, std::move(buffer));
}

msg_ptr make_msg(MessageCreator& builder) {
  auto len = builder.GetSize();
  auto mem = builder.ReleaseBufferPointer();
  builder.Clear();
  return std::make_shared<message>(len, std::move(mem));
}

msg_ptr make_msg(void* buf, size_t len) {
  flatbuffers::unique_ptr_t mem(static_cast<uint8_t*>(operator new(len)),
                                std::default_delete<uint8_t>());
  std::memcpy(mem.get(), buf, len);

  flatbuffers::Verifier verifier(mem.get(), len);
  if (!VerifyMessageBuffer(verifier)) {
    throw boost::system::system_error(error::malformed_msg);
  }

  return std::make_shared<message>(len, std::move(mem));
}

}  // namespace module
}  // namespace motis
