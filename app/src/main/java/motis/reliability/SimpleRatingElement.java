// automatically generated, do not modify

package motis.reliability;

import java.nio.*;
import java.lang.*;
import java.util.*;
import com.google.flatbuffers.*;

public final class SimpleRatingElement extends Table {
  public static SimpleRatingElement getRootAsSimpleRatingElement(ByteBuffer _bb) { return getRootAsSimpleRatingElement(_bb, new SimpleRatingElement()); }
  public static SimpleRatingElement getRootAsSimpleRatingElement(ByteBuffer _bb, SimpleRatingElement obj) { _bb.order(ByteOrder.LITTLE_ENDIAN); return (obj.__init(_bb.getInt(_bb.position()) + _bb.position(), _bb)); }
  public SimpleRatingElement __init(int _i, ByteBuffer _bb) { bb_pos = _i; bb = _bb; return this; }

  public Range range() { return range(new Range()); }
  public Range range(Range obj) { int o = __offset(4); return o != 0 ? obj.__init(o + bb_pos, bb) : null; }
  public SimpleRatingInfo ratings(int j) { return ratings(new SimpleRatingInfo(), j); }
  public SimpleRatingInfo ratings(SimpleRatingInfo obj, int j) { int o = __offset(6); return o != 0 ? obj.__init(__indirect(__vector(o) + j * 4), bb) : null; }
  public int ratingsLength() { int o = __offset(6); return o != 0 ? __vector_len(o) : 0; }

  public static void startSimpleRatingElement(FlatBufferBuilder builder) { builder.startObject(2); }
  public static void addRange(FlatBufferBuilder builder, int rangeOffset) { builder.addStruct(0, rangeOffset, 0); }
  public static void addRatings(FlatBufferBuilder builder, int ratingsOffset) { builder.addOffset(1, ratingsOffset, 0); }
  public static int createRatingsVector(FlatBufferBuilder builder, int[] data) { builder.startVector(4, data.length, 4); for (int i = data.length - 1; i >= 0; i--) builder.addOffset(data[i]); return builder.endVector(); }
  public static void startRatingsVector(FlatBufferBuilder builder, int numElems) { builder.startVector(4, numElems, 4); }
  public static int endSimpleRatingElement(FlatBufferBuilder builder) {
    int o = builder.endObject();
    return o;
  }
};

