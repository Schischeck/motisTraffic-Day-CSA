// automatically generated, do not modify

package motis;

import java.nio.*;
import java.lang.*;
import java.util.*;
import com.google.flatbuffers.*;

public final class MotisSuccess extends Table {
  public static MotisSuccess getRootAsMotisSuccess(ByteBuffer _bb) { return getRootAsMotisSuccess(_bb, new MotisSuccess()); }
  public static MotisSuccess getRootAsMotisSuccess(ByteBuffer _bb, MotisSuccess obj) { _bb.order(ByteOrder.LITTLE_ENDIAN); return (obj.__init(_bb.getInt(_bb.position()) + _bb.position(), _bb)); }
  public MotisSuccess __init(int _i, ByteBuffer _bb) { bb_pos = _i; bb = _bb; return this; }


  public static void startMotisSuccess(FlatBufferBuilder builder) { builder.startObject(0); }
  public static int endMotisSuccess(FlatBufferBuilder builder) {
    int o = builder.endObject();
    return o;
  }
};

