// automatically generated, do not modify

package motis.routing;

import java.nio.*;
import java.lang.*;
import java.util.*;
import com.google.flatbuffers.*;

public final class Range extends Struct {
  public Range __init(int _i, ByteBuffer _bb) { bb_pos = _i; bb = _bb; return this; }

  public short from() { return bb.getShort(bb_pos + 0); }
  public short to() { return bb.getShort(bb_pos + 2); }

  public static int createRange(FlatBufferBuilder builder, short from, short to) {
    builder.prep(2, 4);
    builder.putShort(to);
    builder.putShort(from);
    return builder.offset();
  }
};

