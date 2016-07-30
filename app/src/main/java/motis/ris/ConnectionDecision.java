// automatically generated, do not modify

package motis.ris;

import java.nio.*;
import java.lang.*;
import java.util.*;
import com.google.flatbuffers.*;

public final class ConnectionDecision extends Table {
  public static ConnectionDecision getRootAsConnectionDecision(ByteBuffer _bb) { return getRootAsConnectionDecision(_bb, new ConnectionDecision()); }
  public static ConnectionDecision getRootAsConnectionDecision(ByteBuffer _bb, ConnectionDecision obj) { _bb.order(ByteOrder.LITTLE_ENDIAN); return (obj.__init(_bb.getInt(_bb.position()) + _bb.position(), _bb)); }
  public ConnectionDecision __init(int _i, ByteBuffer _bb) { bb_pos = _i; bb = _bb; return this; }

  public Event base() { return base(new Event()); }
  public Event base(Event obj) { int o = __offset(4); return o != 0 ? obj.__init(__indirect(o + bb_pos), bb) : null; }
  public boolean hold() { int o = __offset(6); return o != 0 ? 0!=bb.get(o + bb_pos) : false; }

  public static int createConnectionDecision(FlatBufferBuilder builder,
      int base,
      boolean hold) {
    builder.startObject(2);
    ConnectionDecision.addBase(builder, base);
    ConnectionDecision.addHold(builder, hold);
    return ConnectionDecision.endConnectionDecision(builder);
  }

  public static void startConnectionDecision(FlatBufferBuilder builder) { builder.startObject(2); }
  public static void addBase(FlatBufferBuilder builder, int baseOffset) { builder.addOffset(0, baseOffset, 0); }
  public static void addHold(FlatBufferBuilder builder, boolean hold) { builder.addBoolean(1, hold, false); }
  public static int endConnectionDecision(FlatBufferBuilder builder) {
    int o = builder.endObject();
    builder.required(o, 4);  // base
    return o;
  }
};

