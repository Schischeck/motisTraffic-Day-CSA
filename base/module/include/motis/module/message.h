#pragma once

#include "flatbuffers/flatbuffers.h"

#include "motis/protocol/Message_generated.h"

namespace flatbuffers {
class Parser;
}

namespace motis {
namespace module {

class MessageCreator : public flatbuffers::FlatBufferBuilder {
public:
  void CreateAndFinish(MsgContent type, flatbuffers::Offset<void> content) {
    Finish(CreateMessage(*this, type, content, 1));
  }
};

struct message {
  message() : len_(0), msg_(nullptr), buf_(nullptr) {}
  message(std::string const& json);
  message(size_t len, flatbuffers::unique_ptr_t mem, Message* msg, void* buf)
      : len_(len), mem_(std::move(mem)), msg_(msg), buf_(buf) {}

  template <typename T>
  T content() {
    return reinterpret_cast<T>(msg_->content());
  }

  MsgContent content_type() const { return msg_->content_type(); }

  operator bool() { return msg_ != nullptr || buf_ == nullptr; }

  std::string to_json() const;

  static void init_parser();

  static std::unique_ptr<flatbuffers::Parser> parser;
  size_t len_;
  flatbuffers::unique_ptr_t mem_;
  Message* msg_;
  void* buf_;
};

typedef std::shared_ptr<message> msg_ptr;

msg_ptr make_msg(std::string const& json);
msg_ptr make_msg(MessageCreator& builder);
msg_ptr make_msg(void* buf, size_t len);

}  // namespace module
}  // namespace motis
