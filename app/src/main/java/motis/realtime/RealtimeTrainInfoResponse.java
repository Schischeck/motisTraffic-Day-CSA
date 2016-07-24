// automatically generated, do not modify

package motis.realtime;

import java.nio.*;
import java.lang.*;
import java.util.*;
import com.google.flatbuffers.*;

/**
 * Sent in response to a RealtimeTrainInfoRequest.
 */
public final class RealtimeTrainInfoResponse extends Table {
  public static RealtimeTrainInfoResponse getRootAsRealtimeTrainInfoResponse(ByteBuffer _bb) { return getRootAsRealtimeTrainInfoResponse(_bb, new RealtimeTrainInfoResponse()); }
  public static RealtimeTrainInfoResponse getRootAsRealtimeTrainInfoResponse(ByteBuffer _bb, RealtimeTrainInfoResponse obj) { _bb.order(ByteOrder.LITTLE_ENDIAN); return (obj.__init(_bb.getInt(_bb.position()) + _bb.position(), _bb)); }
  public RealtimeTrainInfoResponse __init(int _i, ByteBuffer _bb) { bb_pos = _i; bb = _bb; return this; }

  public EventInfo stops(int j) { return stops(new EventInfo(), j); }
  public EventInfo stops(EventInfo obj, int j) { int o = __offset(4); return o != 0 ? obj.__init(__indirect(__vector(o) + j * 4), bb) : null; }
  public int stopsLength() { int o = __offset(4); return o != 0 ? __vector_len(o) : 0; }
  public long routeId() { int o = __offset(6); return o != 0 ? (long)bb.getInt(o + bb_pos) & 0xFFFFFFFFL : 0; }

  public static int createRealtimeTrainInfoResponse(FlatBufferBuilder builder,
      int stops,
      long route_id) {
    builder.startObject(2);
    RealtimeTrainInfoResponse.addRouteId(builder, route_id);
    RealtimeTrainInfoResponse.addStops(builder, stops);
    return RealtimeTrainInfoResponse.endRealtimeTrainInfoResponse(builder);
  }

  public static void startRealtimeTrainInfoResponse(FlatBufferBuilder builder) { builder.startObject(2); }
  public static void addStops(FlatBufferBuilder builder, int stopsOffset) { builder.addOffset(0, stopsOffset, 0); }
  public static int createStopsVector(FlatBufferBuilder builder, int[] data) { builder.startVector(4, data.length, 4); for (int i = data.length - 1; i >= 0; i--) builder.addOffset(data[i]); return builder.endVector(); }
  public static void startStopsVector(FlatBufferBuilder builder, int numElems) { builder.startVector(4, numElems, 4); }
  public static void addRouteId(FlatBufferBuilder builder, long routeId) { builder.addInt(1, (int)(routeId & 0xFFFFFFFFL), 0); }
  public static int endRealtimeTrainInfoResponse(FlatBufferBuilder builder) {
    int o = builder.endObject();
    return o;
  }
};

