// automatically generated, do not modify

package motis.reliability;

import java.nio.*;
import java.lang.*;
import java.util.*;
import com.google.flatbuffers.*;

public final class ReliableRoutingResponse extends Table {
  public static ReliableRoutingResponse getRootAsReliableRoutingResponse(ByteBuffer _bb) { return getRootAsReliableRoutingResponse(_bb, new ReliableRoutingResponse()); }
  public static ReliableRoutingResponse getRootAsReliableRoutingResponse(ByteBuffer _bb, ReliableRoutingResponse obj) { _bb.order(ByteOrder.LITTLE_ENDIAN); return (obj.__init(_bb.getInt(_bb.position()) + _bb.position(), _bb)); }
  public ReliableRoutingResponse __init(int _i, ByteBuffer _bb) { bb_pos = _i; bb = _bb; return this; }

  public ConnectionGraph connectionGraphs(int j) { return connectionGraphs(new ConnectionGraph(), j); }
  public ConnectionGraph connectionGraphs(ConnectionGraph obj, int j) { int o = __offset(4); return o != 0 ? obj.__init(__indirect(__vector(o) + j * 4), bb) : null; }
  public int connectionGraphsLength() { int o = __offset(4); return o != 0 ? __vector_len(o) : 0; }

  public static int createReliableRoutingResponse(FlatBufferBuilder builder,
      int connection_graphs) {
    builder.startObject(1);
    ReliableRoutingResponse.addConnectionGraphs(builder, connection_graphs);
    return ReliableRoutingResponse.endReliableRoutingResponse(builder);
  }

  public static void startReliableRoutingResponse(FlatBufferBuilder builder) { builder.startObject(1); }
  public static void addConnectionGraphs(FlatBufferBuilder builder, int connectionGraphsOffset) { builder.addOffset(0, connectionGraphsOffset, 0); }
  public static int createConnectionGraphsVector(FlatBufferBuilder builder, int[] data) { builder.startVector(4, data.length, 4); for (int i = data.length - 1; i >= 0; i--) builder.addOffset(data[i]); return builder.endVector(); }
  public static void startConnectionGraphsVector(FlatBufferBuilder builder, int numElems) { builder.startVector(4, numElems, 4); }
  public static int endReliableRoutingResponse(FlatBufferBuilder builder) {
    int o = builder.endObject();
    return o;
  }
};

