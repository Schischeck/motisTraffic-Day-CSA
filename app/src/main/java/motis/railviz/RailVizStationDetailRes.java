// automatically generated, do not modify

package motis.railviz;

import java.nio.*;
import java.lang.*;
import java.util.*;
import com.google.flatbuffers.*;

public final class RailVizStationDetailRes extends Table {
  public static RailVizStationDetailRes getRootAsRailVizStationDetailRes(ByteBuffer _bb) { return getRootAsRailVizStationDetailRes(_bb, new RailVizStationDetailRes()); }
  public static RailVizStationDetailRes getRootAsRailVizStationDetailRes(ByteBuffer _bb, RailVizStationDetailRes obj) { _bb.order(ByteOrder.LITTLE_ENDIAN); return (obj.__init(_bb.getInt(_bb.position()) + _bb.position(), _bb)); }
  public RailVizStationDetailRes __init(int _i, ByteBuffer _bb) { bb_pos = _i; bb = _bb; return this; }

  public String name() { int o = __offset(4); return o != 0 ? __string(o + bb_pos) : null; }
  public ByteBuffer nameAsByteBuffer() { return __vector_as_bytebuffer(4, 1); }
  public int stationId() { int o = __offset(6); return o != 0 ? bb.getInt(o + bb_pos) : 0; }
  public RailVizStationDetailResEntry timetable(int j) { return timetable(new RailVizStationDetailResEntry(), j); }
  public RailVizStationDetailResEntry timetable(RailVizStationDetailResEntry obj, int j) { int o = __offset(8); return o != 0 ? obj.__init(__indirect(__vector(o) + j * 4), bb) : null; }
  public int timetableLength() { int o = __offset(8); return o != 0 ? __vector_len(o) : 0; }

  public static int createRailVizStationDetailRes(FlatBufferBuilder builder,
      int name,
      int station_id,
      int timetable) {
    builder.startObject(3);
    RailVizStationDetailRes.addTimetable(builder, timetable);
    RailVizStationDetailRes.addStationId(builder, station_id);
    RailVizStationDetailRes.addName(builder, name);
    return RailVizStationDetailRes.endRailVizStationDetailRes(builder);
  }

  public static void startRailVizStationDetailRes(FlatBufferBuilder builder) { builder.startObject(3); }
  public static void addName(FlatBufferBuilder builder, int nameOffset) { builder.addOffset(0, nameOffset, 0); }
  public static void addStationId(FlatBufferBuilder builder, int stationId) { builder.addInt(1, stationId, 0); }
  public static void addTimetable(FlatBufferBuilder builder, int timetableOffset) { builder.addOffset(2, timetableOffset, 0); }
  public static int createTimetableVector(FlatBufferBuilder builder, int[] data) { builder.startVector(4, data.length, 4); for (int i = data.length - 1; i >= 0; i--) builder.addOffset(data[i]); return builder.endVector(); }
  public static void startTimetableVector(FlatBufferBuilder builder, int numElems) { builder.startVector(4, numElems, 4); }
  public static int endRailVizStationDetailRes(FlatBufferBuilder builder) {
    int o = builder.endObject();
    return o;
  }
};

