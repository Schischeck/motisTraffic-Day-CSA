// automatically generated, do not modify

package motis;

import java.nio.*;
import java.lang.*;
import java.util.*;
import com.google.flatbuffers.*;

public final class Message extends Table {
  public static Message getRootAsMessage(ByteBuffer _bb) { return getRootAsMessage(_bb, new Message()); }
  public static Message getRootAsMessage(ByteBuffer _bb, Message obj) { _bb.order(ByteOrder.LITTLE_ENDIAN); return (obj.__init(_bb.getInt(_bb.position()) + _bb.position(), _bb)); }
  public Message __init(int _i, ByteBuffer _bb) { bb_pos = _i; bb = _bb; return this; }

  public byte contentType() { int o = __offset(4); return o != 0 ? bb.get(o + bb_pos) : 0; }
  public Table content(Table obj) { int o = __offset(6); return o != 0 ? __union(obj, o) : null; }
  public int id() { int o = __offset(8); return o != 0 ? bb.getInt(o + bb_pos) : 0; }

  public static int createMessage(FlatBufferBuilder builder,
      byte content_type,
      int content,
      int id) {
    builder.startObject(3);
    Message.addId(builder, id);
    Message.addContent(builder, content);
    Message.addContentType(builder, content_type);
    return Message.endMessage(builder);
  }

  public static void startMessage(FlatBufferBuilder builder) { builder.startObject(3); }
  public static void addContentType(FlatBufferBuilder builder, byte contentType) { builder.addByte(0, contentType, 0); }
  public static void addContent(FlatBufferBuilder builder, int contentOffset) { builder.addOffset(1, contentOffset, 0); }
  public static void addId(FlatBufferBuilder builder, int id) { builder.addInt(2, id, 0); }
  public static int endMessage(FlatBufferBuilder builder) {
    int o = builder.endObject();
    return o;
  }
  public static void finishMessageBuffer(FlatBufferBuilder builder, int offset) { builder.finish(offset); }
};

