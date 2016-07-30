// automatically generated, do not modify

package motis.reliability;

import java.nio.*;
import java.lang.*;
import java.util.*;
import com.google.flatbuffers.*;

public final class Stop extends Table {
  public static Stop getRootAsStop(ByteBuffer _bb) { return getRootAsStop(_bb, new Stop()); }
  public static Stop getRootAsStop(ByteBuffer _bb, Stop obj) { _bb.order(ByteOrder.LITTLE_ENDIAN); return (obj.__init(_bb.getInt(_bb.position()) + _bb.position(), _bb)); }
  public Stop __init(int _i, ByteBuffer _bb) { bb_pos = _i; bb = _bb; return this; }

  public short index() { int o = __offset(4); return o != 0 ? bb.getShort(o + bb_pos) : 0; }
  public Alternative alternatives(int j) { return alternatives(new Alternative(), j); }
  public Alternative alternatives(Alternative obj, int j) { int o = __offset(6); return o != 0 ? obj.__init(__indirect(__vector(o) + j * 4), bb) : null; }
  public int alternativesLength() { int o = __offset(6); return o != 0 ? __vector_len(o) : 0; }

  public static int createStop(FlatBufferBuilder builder,
      short index,
      int alternatives) {
    builder.startObject(2);
    Stop.addAlternatives(builder, alternatives);
    Stop.addIndex(builder, index);
    return Stop.endStop(builder);
  }

  public static void startStop(FlatBufferBuilder builder) { builder.startObject(2); }
  public static void addIndex(FlatBufferBuilder builder, short index) { builder.addShort(0, index, 0); }
  public static void addAlternatives(FlatBufferBuilder builder, int alternativesOffset) { builder.addOffset(1, alternativesOffset, 0); }
  public static int createAlternativesVector(FlatBufferBuilder builder, int[] data) { builder.startVector(4, data.length, 4); for (int i = data.length - 1; i >= 0; i--) builder.addOffset(data[i]); return builder.endVector(); }
  public static void startAlternativesVector(FlatBufferBuilder builder, int numElems) { builder.startVector(4, numElems, 4); }
  public static int endStop(FlatBufferBuilder builder) {
    int o = builder.endObject();
    return o;
  }
};

