// automatically generated, do not modify

package motis.realtime;

import java.nio.*;
import java.lang.*;
import java.util.*;
import com.google.flatbuffers.*;

/**
 * Request delay information for the train that contains the given event.
 * If single_event is true, only returns delay information for the given event.
 * Otherwise, returns delay informations for all events of the train starting
 * with the given event - i.e. to retrieve delay information for all events
 * of the train, use the first departure event of the train.
 * Returns a RealtimeTrainInfoResponse.
 */
public final class RealtimeTrainInfoRequest extends Table {
  public static RealtimeTrainInfoRequest getRootAsRealtimeTrainInfoRequest(ByteBuffer _bb) { return getRootAsRealtimeTrainInfoRequest(_bb, new RealtimeTrainInfoRequest()); }
  public static RealtimeTrainInfoRequest getRootAsRealtimeTrainInfoRequest(ByteBuffer _bb, RealtimeTrainInfoRequest obj) { _bb.order(ByteOrder.LITTLE_ENDIAN); return (obj.__init(_bb.getInt(_bb.position()) + _bb.position(), _bb)); }
  public RealtimeTrainInfoRequest __init(int _i, ByteBuffer _bb) { bb_pos = _i; bb = _bb; return this; }

  /**
   * First stop of the train for which delay information is desired.
   */
  public GraphTrainEvent firstStop() { return firstStop(new GraphTrainEvent()); }
  public GraphTrainEvent firstStop(GraphTrainEvent obj) { int o = __offset(4); return o != 0 ? obj.__init(__indirect(o + bb_pos), bb) : null; }
  /**
   * If true, only delay information for first_stop is returned,
   * otherwise, delay information for all events of the train starting
   * with first_stop is returned.
   */
  public boolean singleEvent() { int o = __offset(6); return o != 0 ? 0!=bb.get(o + bb_pos) : false; }

  public static int createRealtimeTrainInfoRequest(FlatBufferBuilder builder,
      int first_stop,
      boolean single_event) {
    builder.startObject(2);
    RealtimeTrainInfoRequest.addFirstStop(builder, first_stop);
    RealtimeTrainInfoRequest.addSingleEvent(builder, single_event);
    return RealtimeTrainInfoRequest.endRealtimeTrainInfoRequest(builder);
  }

  public static void startRealtimeTrainInfoRequest(FlatBufferBuilder builder) { builder.startObject(2); }
  public static void addFirstStop(FlatBufferBuilder builder, int firstStopOffset) { builder.addOffset(0, firstStopOffset, 0); }
  public static void addSingleEvent(FlatBufferBuilder builder, boolean singleEvent) { builder.addBoolean(1, singleEvent, false); }
  public static int endRealtimeTrainInfoRequest(FlatBufferBuilder builder) {
    int o = builder.endObject();
    return o;
  }
};

