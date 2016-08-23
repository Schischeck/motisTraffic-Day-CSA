#include "motis/module/message.h"

#include <cstring>
#include <stdexcept>

#include "flatbuffers/idl.h"
#include "flatbuffers/util.h"

#include "motis/core/common/logging.h"
#include "motis/module/error.h"
#include "motis/protocol/resources.h"

#undef GetMessage

using namespace flatbuffers;

namespace motis {
namespace module {

std::unique_ptr<Parser> init_parser() {
  auto parser = std::make_unique<Parser>();
  parser->opts.strict_json = true;
  parser->opts.skip_unexpected_fields_in_json = true;
  int message_symbol_index = -1;
  for (unsigned i = 0; i < number_of_symbols; ++i) {
    if (strcmp(filenames[i], "Message.fbs") == 0) {  // NOLINT
      message_symbol_index = i;
    } else if (!parser->Parse(symbols[i], nullptr, filenames[i])) {  // NOLINT
      printf("error: %s\n", parser->error_.c_str());
      throw std::runtime_error("flatbuffer protocol definitions parser error");
    }
  }
  if (message_symbol_index == -1 ||
      !parser->Parse(symbols[message_symbol_index])) {  // NOLINT
    printf("error: %s\n", parser->error_.c_str());
    throw std::runtime_error("flatbuffer protocol definitions parser error");
  }
  return parser;
}
static std::unique_ptr<Parser> parser = init_parser();

std::string message::to_json() const {
  std::string json;
  flatbuffers::GenerateText(*parser, data(), &json);
  return json;
}

msg_ptr make_msg(std::string const& json) {
  if (json.empty()) {
    LOG(motis::logging::error) << "empty request";
    throw std::system_error(error::unable_to_parse_msg);
  }

  bool parse_ok = parser->Parse(json.c_str());
  if (!parse_ok) {
    LOG(motis::logging::error) << "parse error: " << parser->error_;
    throw std::system_error(error::unable_to_parse_msg);
  }

  flatbuffers::Verifier verifier(parser->builder_.GetBufferPointer(),
                                 parser->builder_.GetSize());
  if (!VerifyMessageBuffer(verifier)) {
    throw std::system_error(error::malformed_msg);
  }
  auto size = parser->builder_.GetSize();
  auto buffer = parser->builder_.ReleaseBufferPointer();

  parser->builder_.Clear();
  return std::make_shared<message>(size, std::move(buffer));
}

msg_ptr make_msg(message_creator& builder) {
  auto len = builder.GetSize();
  auto mem = builder.ReleaseBufferPointer();
  builder.Clear();
  return std::make_shared<message>(len, std::move(mem));
}

msg_ptr make_msg(void const* buf, size_t len) {
  auto msg = std::make_shared<message>(len, buf);

  flatbuffers::Verifier verifier(msg->data(), msg->size());
  if (!VerifyMessageBuffer(verifier)) {
    throw std::system_error(error::malformed_msg);
  }

  return msg;
}

msg_ptr make_no_msg(std::string const& target) {
  message_creator b;
  b.create_and_finish(MsgContent_MotisNoMessage,
                      CreateMotisNoMessage(b).Union(), target);
  return make_msg(b);
}

msg_ptr make_success_msg() {
  message_creator b;
  b.create_and_finish(MsgContent_MotisSuccess, CreateMotisSuccess(b).Union());
  return make_msg(b);
}

msg_ptr make_error_msg(std::error_code const& ec) {
  message_creator b;
  b.create_and_finish(
      MsgContent_MotisError,
      CreateMotisError(b, ec.value(), b.CreateString(ec.category().name()),
                       b.CreateString(ec.message()))
          .Union());
  return make_msg(b);
}

}  // namespace module
}  // namespace motis
