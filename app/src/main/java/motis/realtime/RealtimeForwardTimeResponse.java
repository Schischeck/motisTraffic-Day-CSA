// automatically generated, do not modify

package motis.realtime;

import java.nio.*;
import java.lang.*;
import java.util.*;
import com.google.flatbuffers.*;

public final class RealtimeForwardTimeResponse extends Table {
  public static RealtimeForwardTimeResponse getRootAsRealtimeForwardTimeResponse(ByteBuffer _bb) { return getRootAsRealtimeForwardTimeResponse(_bb, new RealtimeForwardTimeResponse()); }
  public static RealtimeForwardTimeResponse getRootAsRealtimeForwardTimeResponse(ByteBuffer _bb, RealtimeForwardTimeResponse obj) { _bb.order(ByteOrder.LITTLE_ENDIAN); return (obj.__init(_bb.getInt(_bb.position()) + _bb.position(), _bb)); }
  public RealtimeForwardTimeResponse __init(int _i, ByteBuffer _bb) { bb_pos = _i; bb = _bb; return this; }


  public static void startRealtimeForwardTimeResponse(FlatBufferBuilder builder) { builder.startObject(0); }
  public static int endRealtimeForwardTimeResponse(FlatBufferBuilder builder) {
    int o = builder.endObject();
    return o;
  }
};

