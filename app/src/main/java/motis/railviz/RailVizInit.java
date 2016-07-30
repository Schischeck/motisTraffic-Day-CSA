// automatically generated, do not modify

package motis.railviz;

import java.nio.*;
import java.lang.*;
import java.util.*;
import com.google.flatbuffers.*;

public final class RailVizInit extends Table {
  public static RailVizInit getRootAsRailVizInit(ByteBuffer _bb) { return getRootAsRailVizInit(_bb, new RailVizInit()); }
  public static RailVizInit getRootAsRailVizInit(ByteBuffer _bb, RailVizInit obj) { _bb.order(ByteOrder.LITTLE_ENDIAN); return (obj.__init(_bb.getInt(_bb.position()) + _bb.position(), _bb)); }
  public RailVizInit __init(int _i, ByteBuffer _bb) { bb_pos = _i; bb = _bb; return this; }

  public RailVizInitEntry stations(int j) { return stations(new RailVizInitEntry(), j); }
  public RailVizInitEntry stations(RailVizInitEntry obj, int j) { int o = __offset(4); return o != 0 ? obj.__init(__indirect(__vector(o) + j * 4), bb) : null; }
  public int stationsLength() { int o = __offset(4); return o != 0 ? __vector_len(o) : 0; }
  public int scheduleStart() { int o = __offset(6); return o != 0 ? bb.getInt(o + bb_pos) : 0; }
  public int scheduleEnd() { int o = __offset(8); return o != 0 ? bb.getInt(o + bb_pos) : 0; }

  public static int createRailVizInit(FlatBufferBuilder builder,
      int stations,
      int schedule_start,
      int schedule_end) {
    builder.startObject(3);
    RailVizInit.addScheduleEnd(builder, schedule_end);
    RailVizInit.addScheduleStart(builder, schedule_start);
    RailVizInit.addStations(builder, stations);
    return RailVizInit.endRailVizInit(builder);
  }

  public static void startRailVizInit(FlatBufferBuilder builder) { builder.startObject(3); }
  public static void addStations(FlatBufferBuilder builder, int stationsOffset) { builder.addOffset(0, stationsOffset, 0); }
  public static int createStationsVector(FlatBufferBuilder builder, int[] data) { builder.startVector(4, data.length, 4); for (int i = data.length - 1; i >= 0; i--) builder.addOffset(data[i]); return builder.endVector(); }
  public static void startStationsVector(FlatBufferBuilder builder, int numElems) { builder.startVector(4, numElems, 4); }
  public static void addScheduleStart(FlatBufferBuilder builder, int scheduleStart) { builder.addInt(1, scheduleStart, 0); }
  public static void addScheduleEnd(FlatBufferBuilder builder, int scheduleEnd) { builder.addInt(2, scheduleEnd, 0); }
  public static int endRailVizInit(FlatBufferBuilder builder) {
    int o = builder.endObject();
    return o;
  }
};

