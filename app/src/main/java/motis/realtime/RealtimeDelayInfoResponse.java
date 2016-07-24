// automatically generated, do not modify

package motis.realtime;

import java.nio.*;
import java.lang.*;
import java.util.*;
import com.google.flatbuffers.*;

/**
 * Sent in response to a RealtimeDelayInfoRequest.
 */
public final class RealtimeDelayInfoResponse extends Table {
  public static RealtimeDelayInfoResponse getRootAsRealtimeDelayInfoResponse(ByteBuffer _bb) { return getRootAsRealtimeDelayInfoResponse(_bb, new RealtimeDelayInfoResponse()); }
  public static RealtimeDelayInfoResponse getRootAsRealtimeDelayInfoResponse(ByteBuffer _bb, RealtimeDelayInfoResponse obj) { _bb.order(ByteOrder.LITTLE_ENDIAN); return (obj.__init(_bb.getInt(_bb.position()) + _bb.position(), _bb)); }
  public RealtimeDelayInfoResponse __init(int _i, ByteBuffer _bb) { bb_pos = _i; bb = _bb; return this; }

  public DelayInfo delayInfos(int j) { return delayInfos(new DelayInfo(), j); }
  public DelayInfo delayInfos(DelayInfo obj, int j) { int o = __offset(4); return o != 0 ? obj.__init(__indirect(__vector(o) + j * 4), bb) : null; }
  public int delayInfosLength() { int o = __offset(4); return o != 0 ? __vector_len(o) : 0; }

  public static int createRealtimeDelayInfoResponse(FlatBufferBuilder builder,
      int delay_infos) {
    builder.startObject(1);
    RealtimeDelayInfoResponse.addDelayInfos(builder, delay_infos);
    return RealtimeDelayInfoResponse.endRealtimeDelayInfoResponse(builder);
  }

  public static void startRealtimeDelayInfoResponse(FlatBufferBuilder builder) { builder.startObject(1); }
  public static void addDelayInfos(FlatBufferBuilder builder, int delayInfosOffset) { builder.addOffset(0, delayInfosOffset, 0); }
  public static int createDelayInfosVector(FlatBufferBuilder builder, int[] data) { builder.startVector(4, data.length, 4); for (int i = data.length - 1; i >= 0; i--) builder.addOffset(data[i]); return builder.endVector(); }
  public static void startDelayInfosVector(FlatBufferBuilder builder, int numElems) { builder.startVector(4, numElems, 4); }
  public static int endRealtimeDelayInfoResponse(FlatBufferBuilder builder) {
    int o = builder.endObject();
    return o;
  }
};

