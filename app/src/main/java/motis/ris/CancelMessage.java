// automatically generated, do not modify

package motis.ris;

import java.nio.*;
import java.lang.*;
import java.util.*;
import com.google.flatbuffers.*;

public final class CancelMessage extends Table {
  public static CancelMessage getRootAsCancelMessage(ByteBuffer _bb) { return getRootAsCancelMessage(_bb, new CancelMessage()); }
  public static CancelMessage getRootAsCancelMessage(ByteBuffer _bb, CancelMessage obj) { _bb.order(ByteOrder.LITTLE_ENDIAN); return (obj.__init(_bb.getInt(_bb.position()) + _bb.position(), _bb)); }
  public CancelMessage __init(int _i, ByteBuffer _bb) { bb_pos = _i; bb = _bb; return this; }

  public Event events(int j) { return events(new Event(), j); }
  public Event events(Event obj, int j) { int o = __offset(4); return o != 0 ? obj.__init(__indirect(__vector(o) + j * 4), bb) : null; }
  public int eventsLength() { int o = __offset(4); return o != 0 ? __vector_len(o) : 0; }

  public static int createCancelMessage(FlatBufferBuilder builder,
      int events) {
    builder.startObject(1);
    CancelMessage.addEvents(builder, events);
    return CancelMessage.endCancelMessage(builder);
  }

  public static void startCancelMessage(FlatBufferBuilder builder) { builder.startObject(1); }
  public static void addEvents(FlatBufferBuilder builder, int eventsOffset) { builder.addOffset(0, eventsOffset, 0); }
  public static int createEventsVector(FlatBufferBuilder builder, int[] data) { builder.startVector(4, data.length, 4); for (int i = data.length - 1; i >= 0; i--) builder.addOffset(data[i]); return builder.endVector(); }
  public static void startEventsVector(FlatBufferBuilder builder, int numElems) { builder.startVector(4, numElems, 4); }
  public static int endCancelMessage(FlatBufferBuilder builder) {
    int o = builder.endObject();
    builder.required(o, 4);  // events
    return o;
  }
};

