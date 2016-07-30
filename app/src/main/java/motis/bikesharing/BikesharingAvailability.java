// automatically generated, do not modify

package motis.bikesharing;

import java.nio.*;
import java.lang.*;
import java.util.*;
import com.google.flatbuffers.*;

public final class BikesharingAvailability extends Struct {
  public BikesharingAvailability __init(int _i, ByteBuffer _bb) { bb_pos = _i; bb = _bb; return this; }

  public long begin() { return bb.getLong(bb_pos + 0); }
  public long end() { return bb.getLong(bb_pos + 8); }
  public double value() { return bb.getDouble(bb_pos + 16); }

  public static int createBikesharingAvailability(FlatBufferBuilder builder, long begin, long end, double value) {
    builder.prep(8, 24);
    builder.putDouble(value);
    builder.putLong(end);
    builder.putLong(begin);
    return builder.offset();
  }
};

