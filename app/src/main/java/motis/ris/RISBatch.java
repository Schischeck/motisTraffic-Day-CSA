// automatically generated, do not modify

package motis.ris;

import java.nio.*;
import java.lang.*;
import java.util.*;
import com.google.flatbuffers.*;

public final class RISBatch extends Table {
  public static RISBatch getRootAsRISBatch(ByteBuffer _bb) { return getRootAsRISBatch(_bb, new RISBatch()); }
  public static RISBatch getRootAsRISBatch(ByteBuffer _bb, RISBatch obj) { _bb.order(ByteOrder.LITTLE_ENDIAN); return (obj.__init(_bb.getInt(_bb.position()) + _bb.position(), _bb)); }
  public RISBatch __init(int _i, ByteBuffer _bb) { bb_pos = _i; bb = _bb; return this; }

  public MessageHolder messages(int j) { return messages(new MessageHolder(), j); }
  public MessageHolder messages(MessageHolder obj, int j) { int o = __offset(4); return o != 0 ? obj.__init(__indirect(__vector(o) + j * 4), bb) : null; }
  public int messagesLength() { int o = __offset(4); return o != 0 ? __vector_len(o) : 0; }

  public static int createRISBatch(FlatBufferBuilder builder,
      int messages) {
    builder.startObject(1);
    RISBatch.addMessages(builder, messages);
    return RISBatch.endRISBatch(builder);
  }

  public static void startRISBatch(FlatBufferBuilder builder) { builder.startObject(1); }
  public static void addMessages(FlatBufferBuilder builder, int messagesOffset) { builder.addOffset(0, messagesOffset, 0); }
  public static int createMessagesVector(FlatBufferBuilder builder, int[] data) { builder.startVector(4, data.length, 4); for (int i = data.length - 1; i >= 0; i--) builder.addOffset(data[i]); return builder.endVector(); }
  public static void startMessagesVector(FlatBufferBuilder builder, int numElems) { builder.startVector(4, numElems, 4); }
  public static int endRISBatch(FlatBufferBuilder builder) {
    int o = builder.endObject();
    builder.required(o, 4);  // messages
    return o;
  }
  public static void finishRISBatchBuffer(FlatBufferBuilder builder, int offset) { builder.finish(offset); }
};

