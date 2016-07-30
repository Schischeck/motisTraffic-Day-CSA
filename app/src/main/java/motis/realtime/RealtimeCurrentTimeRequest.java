// automatically generated, do not modify

package motis.realtime;

import java.nio.*;
import java.lang.*;
import java.util.*;
import com.google.flatbuffers.*;

/**
 * Send a RealtimeCurrentTimeRequest to request information about the current
 * time of the graph, i.e. the timestamp of the last realtime message that
 * was processed.
 * Returns a RealtimeCurrentTimeResponse.
 */
public final class RealtimeCurrentTimeRequest extends Table {
  public static RealtimeCurrentTimeRequest getRootAsRealtimeCurrentTimeRequest(ByteBuffer _bb) { return getRootAsRealtimeCurrentTimeRequest(_bb, new RealtimeCurrentTimeRequest()); }
  public static RealtimeCurrentTimeRequest getRootAsRealtimeCurrentTimeRequest(ByteBuffer _bb, RealtimeCurrentTimeRequest obj) { _bb.order(ByteOrder.LITTLE_ENDIAN); return (obj.__init(_bb.getInt(_bb.position()) + _bb.position(), _bb)); }
  public RealtimeCurrentTimeRequest __init(int _i, ByteBuffer _bb) { bb_pos = _i; bb = _bb; return this; }


  public static void startRealtimeCurrentTimeRequest(FlatBufferBuilder builder) { builder.startObject(0); }
  public static int endRealtimeCurrentTimeRequest(FlatBufferBuilder builder) {
    int o = builder.endObject();
    return o;
  }
};

