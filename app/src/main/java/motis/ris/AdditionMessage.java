// automatically generated, do not modify

package motis.ris;

import java.nio.*;
import java.lang.*;
import java.util.*;
import com.google.flatbuffers.*;

public final class AdditionMessage extends Table {
  public static AdditionMessage getRootAsAdditionMessage(ByteBuffer _bb) { return getRootAsAdditionMessage(_bb, new AdditionMessage()); }
  public static AdditionMessage getRootAsAdditionMessage(ByteBuffer _bb, AdditionMessage obj) { _bb.order(ByteOrder.LITTLE_ENDIAN); return (obj.__init(_bb.getInt(_bb.position()) + _bb.position(), _bb)); }
  public AdditionMessage __init(int _i, ByteBuffer _bb) { bb_pos = _i; bb = _bb; return this; }

  public AdditionalEvent events(int j) { return events(new AdditionalEvent(), j); }
  public AdditionalEvent events(AdditionalEvent obj, int j) { int o = __offset(4); return o != 0 ? obj.__init(__indirect(__vector(o) + j * 4), bb) : null; }
  public int eventsLength() { int o = __offset(4); return o != 0 ? __vector_len(o) : 0; }

  public static int createAdditionMessage(FlatBufferBuilder builder,
      int events) {
    builder.startObject(1);
    AdditionMessage.addEvents(builder, events);
    return AdditionMessage.endAdditionMessage(builder);
  }

  public static void startAdditionMessage(FlatBufferBuilder builder) { builder.startObject(1); }
  public static void addEvents(FlatBufferBuilder builder, int eventsOffset) { builder.addOffset(0, eventsOffset, 0); }
  public static int createEventsVector(FlatBufferBuilder builder, int[] data) { builder.startVector(4, data.length, 4); for (int i = data.length - 1; i >= 0; i--) builder.addOffset(data[i]); return builder.endVector(); }
  public static void startEventsVector(FlatBufferBuilder builder, int numElems) { builder.startVector(4, numElems, 4); }
  public static int endAdditionMessage(FlatBufferBuilder builder) {
    int o = builder.endObject();
    return o;
  }
};

