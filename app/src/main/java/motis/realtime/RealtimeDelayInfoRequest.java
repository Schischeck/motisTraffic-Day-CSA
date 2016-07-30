// automatically generated, do not modify

package motis.realtime;

import java.nio.*;
import java.lang.*;
import java.util.*;
import com.google.flatbuffers.*;

public final class RealtimeDelayInfoRequest extends Table {
  public static RealtimeDelayInfoRequest getRootAsRealtimeDelayInfoRequest(ByteBuffer _bb) { return getRootAsRealtimeDelayInfoRequest(_bb, new RealtimeDelayInfoRequest()); }
  public static RealtimeDelayInfoRequest getRootAsRealtimeDelayInfoRequest(ByteBuffer _bb, RealtimeDelayInfoRequest obj) { _bb.order(ByteOrder.LITTLE_ENDIAN); return (obj.__init(_bb.getInt(_bb.position()) + _bb.position(), _bb)); }
  public RealtimeDelayInfoRequest __init(int _i, ByteBuffer _bb) { bb_pos = _i; bb = _bb; return this; }


  public static void startRealtimeDelayInfoRequest(FlatBufferBuilder builder) { builder.startObject(0); }
  public static int endRealtimeDelayInfoRequest(FlatBufferBuilder builder) {
    int o = builder.endObject();
    return o;
  }
};

