// automatically generated, do not modify

package motis.routing;

import java.nio.*;
import java.lang.*;
import java.util.*;
import com.google.flatbuffers.*;

public final class Interval extends Struct {
  public Interval __init(int _i, ByteBuffer _bb) { bb_pos = _i; bb = _bb; return this; }

  public long begin() { return bb.getLong(bb_pos + 0); }
  public long end() { return bb.getLong(bb_pos + 8); }

  public static int createInterval(FlatBufferBuilder builder, long begin, long end) {
    builder.prep(8, 16);
    builder.putLong(end);
    builder.putLong(begin);
    return builder.offset();
  }
};

