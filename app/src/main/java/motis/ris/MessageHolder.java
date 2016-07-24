// automatically generated, do not modify

package motis.ris;

import java.nio.*;
import java.lang.*;
import java.util.*;
import com.google.flatbuffers.*;

public final class MessageHolder extends Table {
  public static MessageHolder getRootAsMessageHolder(ByteBuffer _bb) { return getRootAsMessageHolder(_bb, new MessageHolder()); }
  public static MessageHolder getRootAsMessageHolder(ByteBuffer _bb, MessageHolder obj) { _bb.order(ByteOrder.LITTLE_ENDIAN); return (obj.__init(_bb.getInt(_bb.position()) + _bb.position(), _bb)); }
  public MessageHolder __init(int _i, ByteBuffer _bb) { bb_pos = _i; bb = _bb; return this; }

  public int message(int j) { int o = __offset(4); return o != 0 ? bb.get(__vector(o) + j * 1) & 0xFF : 0; }
  public int messageLength() { int o = __offset(4); return o != 0 ? __vector_len(o) : 0; }
  public ByteBuffer messageAsByteBuffer() { return __vector_as_bytebuffer(4, 1); }

  public static int createMessageHolder(FlatBufferBuilder builder,
      int message) {
    builder.startObject(1);
    MessageHolder.addMessage(builder, message);
    return MessageHolder.endMessageHolder(builder);
  }

  public static void startMessageHolder(FlatBufferBuilder builder) { builder.startObject(1); }
  public static void addMessage(FlatBufferBuilder builder, int messageOffset) { builder.addOffset(0, messageOffset, 0); }
  public static int createMessageVector(FlatBufferBuilder builder, byte[] data) { builder.startVector(1, data.length, 1); for (int i = data.length - 1; i >= 0; i--) builder.addByte(data[i]); return builder.endVector(); }
  public static void startMessageVector(FlatBufferBuilder builder, int numElems) { builder.startVector(1, numElems, 1); }
  public static int endMessageHolder(FlatBufferBuilder builder) {
    int o = builder.endObject();
    return o;
  }
};

