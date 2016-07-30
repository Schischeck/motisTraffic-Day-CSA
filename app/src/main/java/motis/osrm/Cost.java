// automatically generated, do not modify

package motis.osrm;

import java.nio.*;
import java.lang.*;
import java.util.*;
import com.google.flatbuffers.*;

public final class Cost extends Struct {
  public Cost __init(int _i, ByteBuffer _bb) { bb_pos = _i; bb = _bb; return this; }

  public int time() { return bb.getInt(bb_pos + 0); }
  public double distance() { return bb.getDouble(bb_pos + 8); }

  public static int createCost(FlatBufferBuilder builder, int time, double distance) {
    builder.prep(8, 16);
    builder.putDouble(distance);
    builder.pad(4);
    builder.putInt(time);
    return builder.offset();
  }
};

