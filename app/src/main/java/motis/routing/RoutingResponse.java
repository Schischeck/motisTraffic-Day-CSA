// automatically generated, do not modify

package motis.routing;

import java.nio.*;
import java.lang.*;
import java.util.*;
import com.google.flatbuffers.*;

public final class RoutingResponse extends Table {
  public static RoutingResponse getRootAsRoutingResponse(ByteBuffer _bb) { return getRootAsRoutingResponse(_bb, new RoutingResponse()); }
  public static RoutingResponse getRootAsRoutingResponse(ByteBuffer _bb, RoutingResponse obj) { _bb.order(ByteOrder.LITTLE_ENDIAN); return (obj.__init(_bb.getInt(_bb.position()) + _bb.position(), _bb)); }
  public RoutingResponse __init(int _i, ByteBuffer _bb) { bb_pos = _i; bb = _bb; return this; }

  public Connection connections(int j) { return connections(new Connection(), j); }
  public Connection connections(Connection obj, int j) { int o = __offset(4); return o != 0 ? obj.__init(__indirect(__vector(o) + j * 4), bb) : null; }
  public int connectionsLength() { int o = __offset(4); return o != 0 ? __vector_len(o) : 0; }

  public static int createRoutingResponse(FlatBufferBuilder builder,
      int connections) {
    builder.startObject(1);
    RoutingResponse.addConnections(builder, connections);
    return RoutingResponse.endRoutingResponse(builder);
  }

  public static void startRoutingResponse(FlatBufferBuilder builder) { builder.startObject(1); }
  public static void addConnections(FlatBufferBuilder builder, int connectionsOffset) { builder.addOffset(0, connectionsOffset, 0); }
  public static int createConnectionsVector(FlatBufferBuilder builder, int[] data) { builder.startVector(4, data.length, 4); for (int i = data.length - 1; i >= 0; i--) builder.addOffset(data[i]); return builder.endVector(); }
  public static void startConnectionsVector(FlatBufferBuilder builder, int numElems) { builder.startVector(4, numElems, 4); }
  public static int endRoutingResponse(FlatBufferBuilder builder) {
    int o = builder.endObject();
    return o;
  }
};

