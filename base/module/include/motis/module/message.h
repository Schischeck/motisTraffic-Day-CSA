#pragma once

#include "flatbuffers/flatbuffers.h"

#include "motis/protocol/Message_generated.h"

#include "motis/core/common/typed_flatbuffer.h"

#include "motis/module/error.h"

namespace flatbuffers {
class Parser;
}

namespace motis {
namespace module {

class MessageCreator : public flatbuffers::FlatBufferBuilder {
public:
  void CreateAndFinish(MsgContent type, flatbuffers::Offset<void> content,
                       std::string const& target = "") {
    Finish(CreateMessage(*this, CreateDestination(*this, DestinationType_Module,
                                                  CreateString(target)),
                         type, content, 1));
  }
};

struct message : public typed_flatbuffer<Message> {
  message() : typed_flatbuffer(0, nullptr) {}
  message(size_t len, flatbuffers::unique_ptr_t mem)
      : typed_flatbuffer(len, std::move(mem)) {}
  message(size_t len, void* ptr) : typed_flatbuffer(len, ptr) {}

  int id() const { return get()->id(); }

  std::string to_json() const;
};

typedef std::shared_ptr<message> msg_ptr;

msg_ptr make_msg(std::string const& json);
msg_ptr make_msg(MessageCreator& builder);
msg_ptr make_msg(void* buf, size_t len);

msg_ptr make_no_msg(std::string const& target = "");
msg_ptr make_success_msg();
msg_ptr make_error_msg(boost::system::error_code const&);

template <typename T>
inline T const* motis_content_(msg_ptr const& msg, MsgContent content_type) {
  if (msg->get()->content_type() != content_type) {
    throw boost::system::system_error(error::unexpected_message_type);
  }
  return reinterpret_cast<T const*>(msg->get()->content());
}

#define motis_content(content_type, msg) \
  motis::module::motis_content_<content_type>(msg, MsgContent_##content_type)

}  // namespace module
}  // namespace motis
