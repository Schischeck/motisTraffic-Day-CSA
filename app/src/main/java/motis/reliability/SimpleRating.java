// automatically generated, do not modify

package motis.reliability;

import java.nio.*;
import java.lang.*;
import java.util.*;
import com.google.flatbuffers.*;

public final class SimpleRating extends Table {
  public static SimpleRating getRootAsSimpleRating(ByteBuffer _bb) { return getRootAsSimpleRating(_bb, new SimpleRating()); }
  public static SimpleRating getRootAsSimpleRating(ByteBuffer _bb, SimpleRating obj) { _bb.order(ByteOrder.LITTLE_ENDIAN); return (obj.__init(_bb.getInt(_bb.position()) + _bb.position(), _bb)); }
  public SimpleRating __init(int _i, ByteBuffer _bb) { bb_pos = _i; bb = _bb; return this; }

  public SimpleRatingElement ratingElements(int j) { return ratingElements(new SimpleRatingElement(), j); }
  public SimpleRatingElement ratingElements(SimpleRatingElement obj, int j) { int o = __offset(4); return o != 0 ? obj.__init(__indirect(__vector(o) + j * 4), bb) : null; }
  public int ratingElementsLength() { int o = __offset(4); return o != 0 ? __vector_len(o) : 0; }
  public float connectionRating() { int o = __offset(6); return o != 0 ? bb.getFloat(o + bb_pos) : 0; }

  public static int createSimpleRating(FlatBufferBuilder builder,
      int rating_elements,
      float connection_rating) {
    builder.startObject(2);
    SimpleRating.addConnectionRating(builder, connection_rating);
    SimpleRating.addRatingElements(builder, rating_elements);
    return SimpleRating.endSimpleRating(builder);
  }

  public static void startSimpleRating(FlatBufferBuilder builder) { builder.startObject(2); }
  public static void addRatingElements(FlatBufferBuilder builder, int ratingElementsOffset) { builder.addOffset(0, ratingElementsOffset, 0); }
  public static int createRatingElementsVector(FlatBufferBuilder builder, int[] data) { builder.startVector(4, data.length, 4); for (int i = data.length - 1; i >= 0; i--) builder.addOffset(data[i]); return builder.endVector(); }
  public static void startRatingElementsVector(FlatBufferBuilder builder, int numElems) { builder.startVector(4, numElems, 4); }
  public static void addConnectionRating(FlatBufferBuilder builder, float connectionRating) { builder.addFloat(1, connectionRating, 0); }
  public static int endSimpleRating(FlatBufferBuilder builder) {
    int o = builder.endObject();
    return o;
  }
};

