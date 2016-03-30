#pragma once

#include "flatbuffers/flatbuffers.h"

#include "motis/protocol/Message_generated.h"

#include "motis/core/common/typed_flatbuffer.h"

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

  template <typename T>
  T content() {
    return reinterpret_cast<T>(get()->content());
  }

  MsgContent content_type() const { return get()->content_type(); }

  std::string to_json() const;
};

typedef std::shared_ptr<message> msg_ptr;

msg_ptr make_msg(std::string const& json);
msg_ptr make_msg(MessageCreator& builder);
msg_ptr make_msg(void* buf, size_t len);

}  // namespace module
}  // namespace motis
