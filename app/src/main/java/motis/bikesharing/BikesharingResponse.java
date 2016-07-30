// automatically generated, do not modify

package motis.bikesharing;

import java.nio.*;
import java.lang.*;
import java.util.*;
import com.google.flatbuffers.*;

public final class BikesharingResponse extends Table {
  public static BikesharingResponse getRootAsBikesharingResponse(ByteBuffer _bb) { return getRootAsBikesharingResponse(_bb, new BikesharingResponse()); }
  public static BikesharingResponse getRootAsBikesharingResponse(ByteBuffer _bb, BikesharingResponse obj) { _bb.order(ByteOrder.LITTLE_ENDIAN); return (obj.__init(_bb.getInt(_bb.position()) + _bb.position(), _bb)); }
  public BikesharingResponse __init(int _i, ByteBuffer _bb) { bb_pos = _i; bb = _bb; return this; }

  public BikesharingEdge departureEdges(int j) { return departureEdges(new BikesharingEdge(), j); }
  public BikesharingEdge departureEdges(BikesharingEdge obj, int j) { int o = __offset(4); return o != 0 ? obj.__init(__indirect(__vector(o) + j * 4), bb) : null; }
  public int departureEdgesLength() { int o = __offset(4); return o != 0 ? __vector_len(o) : 0; }
  public BikesharingEdge arrivalEdges(int j) { return arrivalEdges(new BikesharingEdge(), j); }
  public BikesharingEdge arrivalEdges(BikesharingEdge obj, int j) { int o = __offset(6); return o != 0 ? obj.__init(__indirect(__vector(o) + j * 4), bb) : null; }
  public int arrivalEdgesLength() { int o = __offset(6); return o != 0 ? __vector_len(o) : 0; }

  public static int createBikesharingResponse(FlatBufferBuilder builder,
      int departure_edges,
      int arrival_edges) {
    builder.startObject(2);
    BikesharingResponse.addArrivalEdges(builder, arrival_edges);
    BikesharingResponse.addDepartureEdges(builder, departure_edges);
    return BikesharingResponse.endBikesharingResponse(builder);
  }

  public static void startBikesharingResponse(FlatBufferBuilder builder) { builder.startObject(2); }
  public static void addDepartureEdges(FlatBufferBuilder builder, int departureEdgesOffset) { builder.addOffset(0, departureEdgesOffset, 0); }
  public static int createDepartureEdgesVector(FlatBufferBuilder builder, int[] data) { builder.startVector(4, data.length, 4); for (int i = data.length - 1; i >= 0; i--) builder.addOffset(data[i]); return builder.endVector(); }
  public static void startDepartureEdgesVector(FlatBufferBuilder builder, int numElems) { builder.startVector(4, numElems, 4); }
  public static void addArrivalEdges(FlatBufferBuilder builder, int arrivalEdgesOffset) { builder.addOffset(1, arrivalEdgesOffset, 0); }
  public static int createArrivalEdgesVector(FlatBufferBuilder builder, int[] data) { builder.startVector(4, data.length, 4); for (int i = data.length - 1; i >= 0; i--) builder.addOffset(data[i]); return builder.endVector(); }
  public static void startArrivalEdgesVector(FlatBufferBuilder builder, int numElems) { builder.startVector(4, numElems, 4); }
  public static int endBikesharingResponse(FlatBufferBuilder builder) {
    int o = builder.endObject();
    return o;
  }
};

