// automatically generated by the FlatBuffers compiler, do not modify

package motis.routing;

import java.nio.*;
import java.lang.*;
import java.util.*;
import com.google.flatbuffers.*;

@SuppressWarnings("unused")
public final class RoutingRequest extends Table {
  public static RoutingRequest getRootAsRoutingRequest(ByteBuffer _bb) { return getRootAsRoutingRequest(_bb, new RoutingRequest()); }
  public static RoutingRequest getRootAsRoutingRequest(ByteBuffer _bb, RoutingRequest obj) { _bb.order(ByteOrder.LITTLE_ENDIAN); return (obj.__init(_bb.getInt(_bb.position()) + _bb.position(), _bb)); }
  public RoutingRequest __init(int _i, ByteBuffer _bb) { bb_pos = _i; bb = _bb; return this; }

  public byte startType() { int o = __offset(4); return o != 0 ? bb.get(o + bb_pos) : 0; }
  public Table start(Table obj) { int o = __offset(6); return o != 0 ? __union(obj, o) : null; }
  public InputStation destination() { return destination(new InputStation()); }
  public InputStation destination(InputStation obj) { int o = __offset(8); return o != 0 ? obj.__init(__indirect(o + bb_pos), bb) : null; }
  public byte searchType() { int o = __offset(10); return o != 0 ? bb.get(o + bb_pos) : 0; }
  public byte searchDir() { int o = __offset(12); return o != 0 ? bb.get(o + bb_pos) : 0; }
  public Via via(int j) { return via(new Via(), j); }
  public Via via(Via obj, int j) { int o = __offset(14); return o != 0 ? obj.__init(__indirect(__vector(o) + j * 4), bb) : null; }
  public int viaLength() { int o = __offset(14); return o != 0 ? __vector_len(o) : 0; }
  public AdditionalEdgeWrapper additionalEdges(int j) { return additionalEdges(new AdditionalEdgeWrapper(), j); }
  public AdditionalEdgeWrapper additionalEdges(AdditionalEdgeWrapper obj, int j) { int o = __offset(16); return o != 0 ? obj.__init(__indirect(__vector(o) + j * 4), bb) : null; }
  public int additionalEdgesLength() { int o = __offset(16); return o != 0 ? __vector_len(o) : 0; }

  public static int createRoutingRequest(FlatBufferBuilder builder,
      byte start_type,
      int startOffset,
      int destinationOffset,
      byte search_type,
      byte search_dir,
      int viaOffset,
      int additional_edgesOffset) {
    builder.startObject(7);
    RoutingRequest.addAdditionalEdges(builder, additional_edgesOffset);
    RoutingRequest.addVia(builder, viaOffset);
    RoutingRequest.addDestination(builder, destinationOffset);
    RoutingRequest.addStart(builder, startOffset);
    RoutingRequest.addSearchDir(builder, search_dir);
    RoutingRequest.addSearchType(builder, search_type);
    RoutingRequest.addStartType(builder, start_type);
    return RoutingRequest.endRoutingRequest(builder);
  }

  public static void startRoutingRequest(FlatBufferBuilder builder) { builder.startObject(7); }
  public static void addStartType(FlatBufferBuilder builder, byte startType) { builder.addByte(0, startType, 0); }
  public static void addStart(FlatBufferBuilder builder, int startOffset) { builder.addOffset(1, startOffset, 0); }
  public static void addDestination(FlatBufferBuilder builder, int destinationOffset) { builder.addOffset(2, destinationOffset, 0); }
  public static void addSearchType(FlatBufferBuilder builder, byte searchType) { builder.addByte(3, searchType, 0); }
  public static void addSearchDir(FlatBufferBuilder builder, byte searchDir) { builder.addByte(4, searchDir, 0); }
  public static void addVia(FlatBufferBuilder builder, int viaOffset) { builder.addOffset(5, viaOffset, 0); }
  public static int createViaVector(FlatBufferBuilder builder, int[] data) { builder.startVector(4, data.length, 4); for (int i = data.length - 1; i >= 0; i--) builder.addOffset(data[i]); return builder.endVector(); }
  public static void startViaVector(FlatBufferBuilder builder, int numElems) { builder.startVector(4, numElems, 4); }
  public static void addAdditionalEdges(FlatBufferBuilder builder, int additionalEdgesOffset) { builder.addOffset(6, additionalEdgesOffset, 0); }
  public static int createAdditionalEdgesVector(FlatBufferBuilder builder, int[] data) { builder.startVector(4, data.length, 4); for (int i = data.length - 1; i >= 0; i--) builder.addOffset(data[i]); return builder.endVector(); }
  public static void startAdditionalEdgesVector(FlatBufferBuilder builder, int numElems) { builder.startVector(4, numElems, 4); }
  public static int endRoutingRequest(FlatBufferBuilder builder) {
    int o = builder.endObject();
    return o;
  }
};

