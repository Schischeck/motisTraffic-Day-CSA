// automatically generated, do not modify

package motis.railviz;

import java.nio.*;
import java.lang.*;
import java.util.*;
import com.google.flatbuffers.*;

public final class RailVizCoordinate extends Struct {
  public RailVizCoordinate __init(int _i, ByteBuffer _bb) { bb_pos = _i; bb = _bb; return this; }

  public double lat() { return bb.getDouble(bb_pos + 0); }
  public double lng() { return bb.getDouble(bb_pos + 8); }

  public static int createRailVizCoordinate(FlatBufferBuilder builder, double lat, double lng) {
    builder.prep(8, 16);
    builder.putDouble(lng);
    builder.putDouble(lat);
    return builder.offset();
  }
};

