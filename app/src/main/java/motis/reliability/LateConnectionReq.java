// automatically generated, do not modify

package motis.reliability;

import java.nio.*;
import java.lang.*;
import java.util.*;
import com.google.flatbuffers.*;

public final class LateConnectionReq extends Table {
  public static LateConnectionReq getRootAsLateConnectionReq(ByteBuffer _bb) { return getRootAsLateConnectionReq(_bb, new LateConnectionReq()); }
  public static LateConnectionReq getRootAsLateConnectionReq(ByteBuffer _bb, LateConnectionReq obj) { _bb.order(ByteOrder.LITTLE_ENDIAN); return (obj.__init(_bb.getInt(_bb.position()) + _bb.position(), _bb)); }
  public LateConnectionReq __init(int _i, ByteBuffer _bb) { bb_pos = _i; bb = _bb; return this; }


  public static void startLateConnectionReq(FlatBufferBuilder builder) { builder.startObject(0); }
  public static int endLateConnectionReq(FlatBufferBuilder builder) {
    int o = builder.endObject();
    return o;
  }
};

