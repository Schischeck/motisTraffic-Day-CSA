// automatically generated, do not modify

package motis.railviz;

import java.nio.*;
import java.lang.*;
import java.util.*;
import com.google.flatbuffers.*;

public final class RailVizRouteAtTimeRes extends Table {
  public static RailVizRouteAtTimeRes getRootAsRailVizRouteAtTimeRes(ByteBuffer _bb) { return getRootAsRailVizRouteAtTimeRes(_bb, new RailVizRouteAtTimeRes()); }
  public static RailVizRouteAtTimeRes getRootAsRailVizRouteAtTimeRes(ByteBuffer _bb, RailVizRouteAtTimeRes obj) { _bb.order(ByteOrder.LITTLE_ENDIAN); return (obj.__init(_bb.getInt(_bb.position()) + _bb.position(), _bb)); }
  public RailVizRouteAtTimeRes __init(int _i, ByteBuffer _bb) { bb_pos = _i; bb = _bb; return this; }

  public String trainNum() { int o = __offset(4); return o != 0 ? __string(o + bb_pos) : null; }
  public ByteBuffer trainNumAsByteBuffer() { return __vector_as_bytebuffer(4, 1); }
  public String trainIdentifier() { int o = __offset(6); return o != 0 ? __string(o + bb_pos) : null; }
  public ByteBuffer trainIdentifierAsByteBuffer() { return __vector_as_bytebuffer(6, 1); }
  public int classz() { int o = __offset(8); return o != 0 ? bb.getInt(o + bb_pos) : -1; }
  public RailVizRouteEntry route(int j) { return route(new RailVizRouteEntry(), j); }
  public RailVizRouteEntry route(RailVizRouteEntry obj, int j) { int o = __offset(10); return o != 0 ? obj.__init(__indirect(__vector(o) + j * 4), bb) : null; }
  public int routeLength() { int o = __offset(10); return o != 0 ? __vector_len(o) : 0; }

  public static int createRailVizRouteAtTimeRes(FlatBufferBuilder builder,
      int train_num,
      int train_identifier,
      int classz,
      int route) {
    builder.startObject(4);
    RailVizRouteAtTimeRes.addRoute(builder, route);
    RailVizRouteAtTimeRes.addClassz(builder, classz);
    RailVizRouteAtTimeRes.addTrainIdentifier(builder, train_identifier);
    RailVizRouteAtTimeRes.addTrainNum(builder, train_num);
    return RailVizRouteAtTimeRes.endRailVizRouteAtTimeRes(builder);
  }

  public static void startRailVizRouteAtTimeRes(FlatBufferBuilder builder) { builder.startObject(4); }
  public static void addTrainNum(FlatBufferBuilder builder, int trainNumOffset) { builder.addOffset(0, trainNumOffset, 0); }
  public static void addTrainIdentifier(FlatBufferBuilder builder, int trainIdentifierOffset) { builder.addOffset(1, trainIdentifierOffset, 0); }
  public static void addClassz(FlatBufferBuilder builder, int classz) { builder.addInt(2, classz, -1); }
  public static void addRoute(FlatBufferBuilder builder, int routeOffset) { builder.addOffset(3, routeOffset, 0); }
  public static int createRouteVector(FlatBufferBuilder builder, int[] data) { builder.startVector(4, data.length, 4); for (int i = data.length - 1; i >= 0; i--) builder.addOffset(data[i]); return builder.endVector(); }
  public static void startRouteVector(FlatBufferBuilder builder, int numElems) { builder.startVector(4, numElems, 4); }
  public static int endRailVizRouteAtTimeRes(FlatBufferBuilder builder) {
    int o = builder.endObject();
    return o;
  }
};

