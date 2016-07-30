// automatically generated, do not modify

package motis.ris;

import java.nio.*;
import java.lang.*;
import java.util.*;
import com.google.flatbuffers.*;

public final class ReroutedEvent extends Table {
  public static ReroutedEvent getRootAsReroutedEvent(ByteBuffer _bb) { return getRootAsReroutedEvent(_bb, new ReroutedEvent()); }
  public static ReroutedEvent getRootAsReroutedEvent(ByteBuffer _bb, ReroutedEvent obj) { _bb.order(ByteOrder.LITTLE_ENDIAN); return (obj.__init(_bb.getInt(_bb.position()) + _bb.position(), _bb)); }
  public ReroutedEvent __init(int _i, ByteBuffer _bb) { bb_pos = _i; bb = _bb; return this; }

  public AdditionalEvent base() { return base(new AdditionalEvent()); }
  public AdditionalEvent base(AdditionalEvent obj) { int o = __offset(4); return o != 0 ? obj.__init(__indirect(o + bb_pos), bb) : null; }
  public boolean status() { int o = __offset(6); return o != 0 ? 0!=bb.get(o + bb_pos) : false; }

  public static int createReroutedEvent(FlatBufferBuilder builder,
      int base,
      boolean status) {
    builder.startObject(2);
    ReroutedEvent.addBase(builder, base);
    ReroutedEvent.addStatus(builder, status);
    return ReroutedEvent.endReroutedEvent(builder);
  }

  public static void startReroutedEvent(FlatBufferBuilder builder) { builder.startObject(2); }
  public static void addBase(FlatBufferBuilder builder, int baseOffset) { builder.addOffset(0, baseOffset, 0); }
  public static void addStatus(FlatBufferBuilder builder, boolean status) { builder.addBoolean(1, status, false); }
  public static int endReroutedEvent(FlatBufferBuilder builder) {
    int o = builder.endObject();
    builder.required(o, 4);  // base
    return o;
  }
};

