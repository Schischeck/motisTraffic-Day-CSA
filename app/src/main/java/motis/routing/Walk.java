// automatically generated, do not modify

package motis.routing;

import java.nio.*;
import java.lang.*;
import java.util.*;
import com.google.flatbuffers.*;

public final class Walk extends Table {
  public static Walk getRootAsWalk(ByteBuffer _bb) { return getRootAsWalk(_bb, new Walk()); }
  public static Walk getRootAsWalk(ByteBuffer _bb, Walk obj) { _bb.order(ByteOrder.LITTLE_ENDIAN); return (obj.__init(_bb.getInt(_bb.position()) + _bb.position(), _bb)); }
  public Walk __init(int _i, ByteBuffer _bb) { bb_pos = _i; bb = _bb; return this; }

  public Range range() { return range(new Range()); }
  public Range range(Range obj) { int o = __offset(4); return o != 0 ? obj.__init(o + bb_pos, bb) : null; }

  public static void startWalk(FlatBufferBuilder builder) { builder.startObject(1); }
  public static void addRange(FlatBufferBuilder builder, int rangeOffset) { builder.addStruct(0, rangeOffset, 0); }
  public static int endWalk(FlatBufferBuilder builder) {
    int o = builder.endObject();
    return o;
  }
};

