// automatically generated, do not modify

package motis.ris;

import java.nio.*;
import java.lang.*;
import java.util.*;
import com.google.flatbuffers.*;

public final class RerouteMessage extends Table {
  public static RerouteMessage getRootAsRerouteMessage(ByteBuffer _bb) { return getRootAsRerouteMessage(_bb, new RerouteMessage()); }
  public static RerouteMessage getRootAsRerouteMessage(ByteBuffer _bb, RerouteMessage obj) { _bb.order(ByteOrder.LITTLE_ENDIAN); return (obj.__init(_bb.getInt(_bb.position()) + _bb.position(), _bb)); }
  public RerouteMessage __init(int _i, ByteBuffer _bb) { bb_pos = _i; bb = _bb; return this; }

  public Event cancelledEvents(int j) { return cancelledEvents(new Event(), j); }
  public Event cancelledEvents(Event obj, int j) { int o = __offset(4); return o != 0 ? obj.__init(__indirect(__vector(o) + j * 4), bb) : null; }
  public int cancelledEventsLength() { int o = __offset(4); return o != 0 ? __vector_len(o) : 0; }
  public ReroutedEvent newEvents(int j) { return newEvents(new ReroutedEvent(), j); }
  public ReroutedEvent newEvents(ReroutedEvent obj, int j) { int o = __offset(6); return o != 0 ? obj.__init(__indirect(__vector(o) + j * 4), bb) : null; }
  public int newEventsLength() { int o = __offset(6); return o != 0 ? __vector_len(o) : 0; }

  public static int createRerouteMessage(FlatBufferBuilder builder,
      int cancelledEvents,
      int newEvents) {
    builder.startObject(2);
    RerouteMessage.addNewEvents(builder, newEvents);
    RerouteMessage.addCancelledEvents(builder, cancelledEvents);
    return RerouteMessage.endRerouteMessage(builder);
  }

  public static void startRerouteMessage(FlatBufferBuilder builder) { builder.startObject(2); }
  public static void addCancelledEvents(FlatBufferBuilder builder, int cancelledEventsOffset) { builder.addOffset(0, cancelledEventsOffset, 0); }
  public static int createCancelledEventsVector(FlatBufferBuilder builder, int[] data) { builder.startVector(4, data.length, 4); for (int i = data.length - 1; i >= 0; i--) builder.addOffset(data[i]); return builder.endVector(); }
  public static void startCancelledEventsVector(FlatBufferBuilder builder, int numElems) { builder.startVector(4, numElems, 4); }
  public static void addNewEvents(FlatBufferBuilder builder, int newEventsOffset) { builder.addOffset(1, newEventsOffset, 0); }
  public static int createNewEventsVector(FlatBufferBuilder builder, int[] data) { builder.startVector(4, data.length, 4); for (int i = data.length - 1; i >= 0; i--) builder.addOffset(data[i]); return builder.endVector(); }
  public static void startNewEventsVector(FlatBufferBuilder builder, int numElems) { builder.startVector(4, numElems, 4); }
  public static int endRerouteMessage(FlatBufferBuilder builder) {
    int o = builder.endObject();
    builder.required(o, 4);  // cancelledEvents
    builder.required(o, 6);  // newEvents
    return o;
  }
};

