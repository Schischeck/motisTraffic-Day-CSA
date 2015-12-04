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
  message() : msg_(nullptr), buf_(nullptr) {}
  message(std::string const& json);
  message(flatbuffers::unique_ptr_t mem, Message* msg, void* buf)
      : mem_(std::move(mem)), msg_(msg), buf_(buf) {}

  template <typename T>
  T content() {
    return reinterpret_cast<T>(msg_->content());
  }

  MsgContent content_type() const { return msg_->content_type(); }

  operator bool() { return msg_ != nullptr || buf_ == nullptr; }

  std::string to_json() const;

  static void init_parser();

  static std::unique_ptr<flatbuffers::Parser> parser;
  flatbuffers::unique_ptr_t mem_;
  Message* msg_;
  void* buf_;
};

typedef std::shared_ptr<message> msg_ptr;

inline msg_ptr make_msg(std::string const& json) {
  return std::make_shared<message>(json);
}

inline msg_ptr make_msg(MessageCreator& builder) {
  auto buf = builder.GetBufferPointer();
  auto msg = GetMutableMessage(buf);
  auto mem = builder.ReleaseBufferPointer();
  builder.Clear();
  return std::make_shared<message>(std::move(mem), msg, buf);
}

}  // namespace module
}  // namespace motis
