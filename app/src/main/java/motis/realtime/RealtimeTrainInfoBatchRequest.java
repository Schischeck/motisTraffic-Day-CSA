// automatically generated, do not modify

package motis.realtime;

import java.nio.*;
import java.lang.*;
import java.util.*;
import com.google.flatbuffers.*;

/**
 * Request delay information for multiple trains
 * Returns a RealtimeTrainInfoBatchResponse
 */
public final class RealtimeTrainInfoBatchRequest extends Table {
  public static RealtimeTrainInfoBatchRequest getRootAsRealtimeTrainInfoBatchRequest(ByteBuffer _bb) { return getRootAsRealtimeTrainInfoBatchRequest(_bb, new RealtimeTrainInfoBatchRequest()); }
  public static RealtimeTrainInfoBatchRequest getRootAsRealtimeTrainInfoBatchRequest(ByteBuffer _bb, RealtimeTrainInfoBatchRequest obj) { _bb.order(ByteOrder.LITTLE_ENDIAN); return (obj.__init(_bb.getInt(_bb.position()) + _bb.position(), _bb)); }
  public RealtimeTrainInfoBatchRequest __init(int _i, ByteBuffer _bb) { bb_pos = _i; bb = _bb; return this; }

  public RealtimeTrainInfoRequest trains(int j) { return trains(new RealtimeTrainInfoRequest(), j); }
  public RealtimeTrainInfoRequest trains(RealtimeTrainInfoRequest obj, int j) { int o = __offset(4); return o != 0 ? obj.__init(__indirect(__vector(o) + j * 4), bb) : null; }
  public int trainsLength() { int o = __offset(4); return o != 0 ? __vector_len(o) : 0; }

  public static int createRealtimeTrainInfoBatchRequest(FlatBufferBuilder builder,
      int trains) {
    builder.startObject(1);
    RealtimeTrainInfoBatchRequest.addTrains(builder, trains);
    return RealtimeTrainInfoBatchRequest.endRealtimeTrainInfoBatchRequest(builder);
  }

  public static void startRealtimeTrainInfoBatchRequest(FlatBufferBuilder builder) { builder.startObject(1); }
  public static void addTrains(FlatBufferBuilder builder, int trainsOffset) { builder.addOffset(0, trainsOffset, 0); }
  public static int createTrainsVector(FlatBufferBuilder builder, int[] data) { builder.startVector(4, data.length, 4); for (int i = data.length - 1; i >= 0; i--) builder.addOffset(data[i]); return builder.endVector(); }
  public static void startTrainsVector(FlatBufferBuilder builder, int numElems) { builder.startVector(4, numElems, 4); }
  public static int endRealtimeTrainInfoBatchRequest(FlatBufferBuilder builder) {
    int o = builder.endObject();
    return o;
  }
};

