// automatically generated by the FlatBuffers compiler, do not modify

package motis.routing;

import java.nio.*;
import java.lang.*;
import java.util.*;
import com.google.flatbuffers.*;

@SuppressWarnings("unused")
public final class PretripStart extends Table {
  public static PretripStart getRootAsPretripStart(ByteBuffer _bb) { return getRootAsPretripStart(_bb, new PretripStart()); }
  public static PretripStart getRootAsPretripStart(ByteBuffer _bb, PretripStart obj) { _bb.order(ByteOrder.LITTLE_ENDIAN); return (obj.__init(_bb.getInt(_bb.position()) + _bb.position(), _bb)); }
  public PretripStart __init(int _i, ByteBuffer _bb) { bb_pos = _i; bb = _bb; return this; }

  public InputStation station() { return station(new InputStation()); }
  public InputStation station(InputStation obj) { int o = __offset(4); return o != 0 ? obj.__init(__indirect(o + bb_pos), bb) : null; }
  public motis.Interval interval() { return interval(new motis.Interval()); }
  public motis.Interval interval(motis.Interval obj) { int o = __offset(6); return o != 0 ? obj.__init(o + bb_pos, bb) : null; }

  public static void startPretripStart(FlatBufferBuilder builder) { builder.startObject(2); }
  public static void addStation(FlatBufferBuilder builder, int stationOffset) { builder.addOffset(0, stationOffset, 0); }
  public static void addInterval(FlatBufferBuilder builder, int intervalOffset) { builder.addStruct(1, intervalOffset, 0); }
  public static int endPretripStart(FlatBufferBuilder builder) {
    int o = builder.endObject();
    return o;
  }
};

