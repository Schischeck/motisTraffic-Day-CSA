// automatically generated, do not modify

package motis.routing;

import java.nio.*;
import java.lang.*;
import java.util.*;
import com.google.flatbuffers.*;

public final class RoutingRequest extends Table {
  public static RoutingRequest getRootAsRoutingRequest(ByteBuffer _bb) { return getRootAsRoutingRequest(_bb, new RoutingRequest()); }
  public static RoutingRequest getRootAsRoutingRequest(ByteBuffer _bb, RoutingRequest obj) { _bb.order(ByteOrder.LITTLE_ENDIAN); return (obj.__init(_bb.getInt(_bb.position()) + _bb.position(), _bb)); }
  public RoutingRequest __init(int _i, ByteBuffer _bb) { bb_pos = _i; bb = _bb; return this; }

  public Interval interval() { return interval(new Interval()); }
  public Interval interval(Interval obj) { int o = __offset(4); return o != 0 ? obj.__init(o + bb_pos, bb) : null; }
  public byte type() { int o = __offset(6); return o != 0 ? bb.get(o + bb_pos) : 0; }
  public byte direction() { int o = __offset(8); return o != 0 ? bb.get(o + bb_pos) : 0; }
  public StationPathElement path(int j) { return path(new StationPathElement(), j); }
  public StationPathElement path(StationPathElement obj, int j) { int o = __offset(10); return o != 0 ? obj.__init(__indirect(__vector(o) + j * 4), bb) : null; }
  public int pathLength() { int o = __offset(10); return o != 0 ? __vector_len(o) : 0; }
  public AdditionalEdgeWrapper additionalEdges(int j) { return additionalEdges(new AdditionalEdgeWrapper(), j); }
  public AdditionalEdgeWrapper additionalEdges(AdditionalEdgeWrapper obj, int j) { int o = __offset(12); return o != 0 ? obj.__init(__indirect(__vector(o) + j * 4), bb) : null; }
  public int additionalEdgesLength() { int o = __offset(12); return o != 0 ? __vector_len(o) : 0; }

  public static void startRoutingRequest(FlatBufferBuilder builder) { builder.startObject(5); }
  public static void addInterval(FlatBufferBuilder builder, int intervalOffset) { builder.addStruct(0, intervalOffset, 0); }
  public static void addType(FlatBufferBuilder builder, byte type) { builder.addByte(1, type, 0); }
  public static void addDirection(FlatBufferBuilder builder, byte direction) { builder.addByte(2, direction, 0); }
  public static void addPath(FlatBufferBuilder builder, int pathOffset) { builder.addOffset(3, pathOffset, 0); }
  public static int createPathVector(FlatBufferBuilder builder, int[] data) { builder.startVector(4, data.length, 4); for (int i = data.length - 1; i >= 0; i--) builder.addOffset(data[i]); return builder.endVector(); }
  public static void startPathVector(FlatBufferBuilder builder, int numElems) { builder.startVector(4, numElems, 4); }
  public static void addAdditionalEdges(FlatBufferBuilder builder, int additionalEdgesOffset) { builder.addOffset(4, additionalEdgesOffset, 0); }
  public static int createAdditionalEdgesVector(FlatBufferBuilder builder, int[] data) { builder.startVector(4, data.length, 4); for (int i = data.length - 1; i >= 0; i--) builder.addOffset(data[i]); return builder.endVector(); }
  public static void startAdditionalEdgesVector(FlatBufferBuilder builder, int numElems) { builder.startVector(4, numElems, 4); }
  public static int endRoutingRequest(FlatBufferBuilder builder) {
    int o = builder.endObject();
    return o;
  }
};

