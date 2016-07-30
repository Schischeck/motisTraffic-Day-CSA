// automatically generated, do not modify

package motis.railviz;

import java.nio.*;
import java.lang.*;
import java.util.*;
import com.google.flatbuffers.*;

public final class RailVizAlltraReq extends Table {
  public static RailVizAlltraReq getRootAsRailVizAlltraReq(ByteBuffer _bb) { return getRootAsRailVizAlltraReq(_bb, new RailVizAlltraReq()); }
  public static RailVizAlltraReq getRootAsRailVizAlltraReq(ByteBuffer _bb, RailVizAlltraReq obj) { _bb.order(ByteOrder.LITTLE_ENDIAN); return (obj.__init(_bb.getInt(_bb.position()) + _bb.position(), _bb)); }
  public RailVizAlltraReq __init(int _i, ByteBuffer _bb) { bb_pos = _i; bb = _bb; return this; }

  public RailVizCoordinate p1() { return p1(new RailVizCoordinate()); }
  public RailVizCoordinate p1(RailVizCoordinate obj) { int o = __offset(4); return o != 0 ? obj.__init(o + bb_pos, bb) : null; }
  public RailVizCoordinate p2() { return p2(new RailVizCoordinate()); }
  public RailVizCoordinate p2(RailVizCoordinate obj) { int o = __offset(6); return o != 0 ? obj.__init(o + bb_pos, bb) : null; }
  public long time() { int o = __offset(8); return o != 0 ? bb.getLong(o + bb_pos) : 0; }
  public boolean withRoutes() { int o = __offset(10); return o != 0 ? 0!=bb.get(o + bb_pos) : false; }

  public static void startRailVizAlltraReq(FlatBufferBuilder builder) { builder.startObject(4); }
  public static void addP1(FlatBufferBuilder builder, int p1Offset) { builder.addStruct(0, p1Offset, 0); }
  public static void addP2(FlatBufferBuilder builder, int p2Offset) { builder.addStruct(1, p2Offset, 0); }
  public static void addTime(FlatBufferBuilder builder, long time) { builder.addLong(2, time, 0); }
  public static void addWithRoutes(FlatBufferBuilder builder, boolean withRoutes) { builder.addBoolean(3, withRoutes, false); }
  public static int endRailVizAlltraReq(FlatBufferBuilder builder) {
    int o = builder.endObject();
    return o;
  }
};

