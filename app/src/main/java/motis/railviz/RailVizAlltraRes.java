// automatically generated, do not modify

package motis.railviz;

import java.nio.*;
import java.lang.*;
import java.util.*;
import com.google.flatbuffers.*;

public final class RailVizAlltraRes extends Table {
  public static RailVizAlltraRes getRootAsRailVizAlltraRes(ByteBuffer _bb) { return getRootAsRailVizAlltraRes(_bb, new RailVizAlltraRes()); }
  public static RailVizAlltraRes getRootAsRailVizAlltraRes(ByteBuffer _bb, RailVizAlltraRes obj) { _bb.order(ByteOrder.LITTLE_ENDIAN); return (obj.__init(_bb.getInt(_bb.position()) + _bb.position(), _bb)); }
  public RailVizAlltraRes __init(int _i, ByteBuffer _bb) { bb_pos = _i; bb = _bb; return this; }

  public RailVizTrain trains(int j) { return trains(new RailVizTrain(), j); }
  public RailVizTrain trains(RailVizTrain obj, int j) { int o = __offset(4); return o != 0 ? obj.__init(__indirect(__vector(o) + j * 4), bb) : null; }
  public int trainsLength() { int o = __offset(4); return o != 0 ? __vector_len(o) : 0; }
  public RailVizAlltraResRoute routes(int j) { return routes(new RailVizAlltraResRoute(), j); }
  public RailVizAlltraResRoute routes(RailVizAlltraResRoute obj, int j) { int o = __offset(6); return o != 0 ? obj.__init(__indirect(__vector(o) + j * 4), bb) : null; }
  public int routesLength() { int o = __offset(6); return o != 0 ? __vector_len(o) : 0; }

  public static int createRailVizAlltraRes(FlatBufferBuilder builder,
      int trains,
      int routes) {
    builder.startObject(2);
    RailVizAlltraRes.addRoutes(builder, routes);
    RailVizAlltraRes.addTrains(builder, trains);
    return RailVizAlltraRes.endRailVizAlltraRes(builder);
  }

  public static void startRailVizAlltraRes(FlatBufferBuilder builder) { builder.startObject(2); }
  public static void addTrains(FlatBufferBuilder builder, int trainsOffset) { builder.addOffset(0, trainsOffset, 0); }
  public static int createTrainsVector(FlatBufferBuilder builder, int[] data) { builder.startVector(4, data.length, 4); for (int i = data.length - 1; i >= 0; i--) builder.addOffset(data[i]); return builder.endVector(); }
  public static void startTrainsVector(FlatBufferBuilder builder, int numElems) { builder.startVector(4, numElems, 4); }
  public static void addRoutes(FlatBufferBuilder builder, int routesOffset) { builder.addOffset(1, routesOffset, 0); }
  public static int createRoutesVector(FlatBufferBuilder builder, int[] data) { builder.startVector(4, data.length, 4); for (int i = data.length - 1; i >= 0; i--) builder.addOffset(data[i]); return builder.endVector(); }
  public static void startRoutesVector(FlatBufferBuilder builder, int numElems) { builder.startVector(4, numElems, 4); }
  public static int endRailVizAlltraRes(FlatBufferBuilder builder) {
    int o = builder.endObject();
    return o;
  }
};

