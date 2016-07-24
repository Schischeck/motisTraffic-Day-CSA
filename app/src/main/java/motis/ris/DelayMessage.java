// automatically generated, do not modify

package motis.ris;

import java.nio.*;
import java.lang.*;
import java.util.*;
import com.google.flatbuffers.*;

public final class DelayMessage extends Table {
  public static DelayMessage getRootAsDelayMessage(ByteBuffer _bb) { return getRootAsDelayMessage(_bb, new DelayMessage()); }
  public static DelayMessage getRootAsDelayMessage(ByteBuffer _bb, DelayMessage obj) { _bb.order(ByteOrder.LITTLE_ENDIAN); return (obj.__init(_bb.getInt(_bb.position()) + _bb.position(), _bb)); }
  public DelayMessage __init(int _i, ByteBuffer _bb) { bb_pos = _i; bb = _bb; return this; }

  public boolean type() { int o = __offset(4); return o != 0 ? 0!=bb.get(o + bb_pos) : false; }
  public UpdatedEvent events(int j) { return events(new UpdatedEvent(), j); }
  public UpdatedEvent events(UpdatedEvent obj, int j) { int o = __offset(6); return o != 0 ? obj.__init(__indirect(__vector(o) + j * 4), bb) : null; }
  public int eventsLength() { int o = __offset(6); return o != 0 ? __vector_len(o) : 0; }

  public static int createDelayMessage(FlatBufferBuilder builder,
      boolean type,
      int events) {
    builder.startObject(2);
    DelayMessage.addEvents(builder, events);
    DelayMessage.addType(builder, type);
    return DelayMessage.endDelayMessage(builder);
  }

  public static void startDelayMessage(FlatBufferBuilder builder) { builder.startObject(2); }
  public static void addType(FlatBufferBuilder builder, boolean type) { builder.addBoolean(0, type, false); }
  public static void addEvents(FlatBufferBuilder builder, int eventsOffset) { builder.addOffset(1, eventsOffset, 0); }
  public static int createEventsVector(FlatBufferBuilder builder, int[] data) { builder.startVector(4, data.length, 4); for (int i = data.length - 1; i >= 0; i--) builder.addOffset(data[i]); return builder.endVector(); }
  public static void startEventsVector(FlatBufferBuilder builder, int numElems) { builder.startVector(4, numElems, 4); }
  public static int endDelayMessage(FlatBufferBuilder builder) {
    int o = builder.endObject();
    builder.required(o, 6);  // events
    return o;
  }
};

