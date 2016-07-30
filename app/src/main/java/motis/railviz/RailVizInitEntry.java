// automatically generated, do not modify

package motis.railviz;

import java.nio.*;
import java.lang.*;
import java.util.*;
import com.google.flatbuffers.*;

public final class RailVizInitEntry extends Table {
  public static RailVizInitEntry getRootAsRailVizInitEntry(ByteBuffer _bb) { return getRootAsRailVizInitEntry(_bb, new RailVizInitEntry()); }
  public static RailVizInitEntry getRootAsRailVizInitEntry(ByteBuffer _bb, RailVizInitEntry obj) { _bb.order(ByteOrder.LITTLE_ENDIAN); return (obj.__init(_bb.getInt(_bb.position()) + _bb.position(), _bb)); }
  public RailVizInitEntry __init(int _i, ByteBuffer _bb) { bb_pos = _i; bb = _bb; return this; }

  public String stationName() { int o = __offset(4); return o != 0 ? __string(o + bb_pos) : null; }
  public ByteBuffer stationNameAsByteBuffer() { return __vector_as_bytebuffer(4, 1); }
  public RailVizCoordinate stationCoord() { return stationCoord(new RailVizCoordinate()); }
  public RailVizCoordinate stationCoord(RailVizCoordinate obj) { int o = __offset(6); return o != 0 ? obj.__init(o + bb_pos, bb) : null; }

  public static void startRailVizInitEntry(FlatBufferBuilder builder) { builder.startObject(2); }
  public static void addStationName(FlatBufferBuilder builder, int stationNameOffset) { builder.addOffset(0, stationNameOffset, 0); }
  public static void addStationCoord(FlatBufferBuilder builder, int stationCoordOffset) { builder.addStruct(1, stationCoordOffset, 0); }
  public static int endRailVizInitEntry(FlatBufferBuilder builder) {
    int o = builder.endObject();
    return o;
  }
};

