// automatically generated, do not modify

package motis.railviz;

import java.nio.*;
import java.lang.*;
import java.util.*;
import com.google.flatbuffers.*;

public final class RailVizAlltraResRoute extends Table {
  public static RailVizAlltraResRoute getRootAsRailVizAlltraResRoute(ByteBuffer _bb) { return getRootAsRailVizAlltraResRoute(_bb, new RailVizAlltraResRoute()); }
  public static RailVizAlltraResRoute getRootAsRailVizAlltraResRoute(ByteBuffer _bb, RailVizAlltraResRoute obj) { _bb.order(ByteOrder.LITTLE_ENDIAN); return (obj.__init(_bb.getInt(_bb.position()) + _bb.position(), _bb)); }
  public RailVizAlltraResRoute __init(int _i, ByteBuffer _bb) { bb_pos = _i; bb = _bb; return this; }

  public long route(int j) { int o = __offset(4); return o != 0 ? (long)bb.getInt(__vector(o) + j * 4) & 0xFFFFFFFFL : 0; }
  public int routeLength() { int o = __offset(4); return o != 0 ? __vector_len(o) : 0; }
  public ByteBuffer routeAsByteBuffer() { return __vector_as_bytebuffer(4, 4); }

  public static int createRailVizAlltraResRoute(FlatBufferBuilder builder,
      int route) {
    builder.startObject(1);
    RailVizAlltraResRoute.addRoute(builder, route);
    return RailVizAlltraResRoute.endRailVizAlltraResRoute(builder);
  }

  public static void startRailVizAlltraResRoute(FlatBufferBuilder builder) { builder.startObject(1); }
  public static void addRoute(FlatBufferBuilder builder, int routeOffset) { builder.addOffset(0, routeOffset, 0); }
  public static int createRouteVector(FlatBufferBuilder builder, int[] data) { builder.startVector(4, data.length, 4); for (int i = data.length - 1; i >= 0; i--) builder.addInt(data[i]); return builder.endVector(); }
  public static void startRouteVector(FlatBufferBuilder builder, int numElems) { builder.startVector(4, numElems, 4); }
  public static int endRailVizAlltraResRoute(FlatBufferBuilder builder) {
    int o = builder.endObject();
    return o;
  }
};

