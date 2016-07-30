// automatically generated, do not modify

package motis.routing;

import java.nio.*;
import java.lang.*;
import java.util.*;
import com.google.flatbuffers.*;

public final class TimeDependentMumoEdge extends Table {
  public static TimeDependentMumoEdge getRootAsTimeDependentMumoEdge(ByteBuffer _bb) { return getRootAsTimeDependentMumoEdge(_bb, new TimeDependentMumoEdge()); }
  public static TimeDependentMumoEdge getRootAsTimeDependentMumoEdge(ByteBuffer _bb, TimeDependentMumoEdge obj) { _bb.order(ByteOrder.LITTLE_ENDIAN); return (obj.__init(_bb.getInt(_bb.position()) + _bb.position(), _bb)); }
  public TimeDependentMumoEdge __init(int _i, ByteBuffer _bb) { bb_pos = _i; bb = _bb; return this; }

  public MumoEdge edge() { return edge(new MumoEdge()); }
  public MumoEdge edge(MumoEdge obj) { int o = __offset(4); return o != 0 ? obj.__init(__indirect(o + bb_pos), bb) : null; }
  public int validFrom() { int o = __offset(6); return o != 0 ? bb.getShort(o + bb_pos) & 0xFFFF : 0; }
  public int validTo() { int o = __offset(8); return o != 0 ? bb.getShort(o + bb_pos) & 0xFFFF : 0; }

  public static int createTimeDependentMumoEdge(FlatBufferBuilder builder,
      int edge,
      int valid_from,
      int valid_to) {
    builder.startObject(3);
    TimeDependentMumoEdge.addEdge(builder, edge);
    TimeDependentMumoEdge.addValidTo(builder, valid_to);
    TimeDependentMumoEdge.addValidFrom(builder, valid_from);
    return TimeDependentMumoEdge.endTimeDependentMumoEdge(builder);
  }

  public static void startTimeDependentMumoEdge(FlatBufferBuilder builder) { builder.startObject(3); }
  public static void addEdge(FlatBufferBuilder builder, int edgeOffset) { builder.addOffset(0, edgeOffset, 0); }
  public static void addValidFrom(FlatBufferBuilder builder, int validFrom) { builder.addShort(1, (short)(validFrom & 0xFFFF), 0); }
  public static void addValidTo(FlatBufferBuilder builder, int validTo) { builder.addShort(2, (short)(validTo & 0xFFFF), 0); }
  public static int endTimeDependentMumoEdge(FlatBufferBuilder builder) {
    int o = builder.endObject();
    return o;
  }
};

