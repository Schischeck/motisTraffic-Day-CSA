// automatically generated, do not modify

package motis.reliability;

import java.nio.*;
import java.lang.*;
import java.util.*;
import com.google.flatbuffers.*;

public final class Rating extends Table {
  public static Rating getRootAsRating(ByteBuffer _bb) { return getRootAsRating(_bb, new Rating()); }
  public static Rating getRootAsRating(ByteBuffer _bb, Rating obj) { _bb.order(ByteOrder.LITTLE_ENDIAN); return (obj.__init(_bb.getInt(_bb.position()) + _bb.position(), _bb)); }
  public Rating __init(int _i, ByteBuffer _bb) { bb_pos = _i; bb = _bb; return this; }

  public RatingElement ratingElements(int j) { return ratingElements(new RatingElement(), j); }
  public RatingElement ratingElements(RatingElement obj, int j) { int o = __offset(4); return o != 0 ? obj.__init(__indirect(__vector(o) + j * 4), bb) : null; }
  public int ratingElementsLength() { int o = __offset(4); return o != 0 ? __vector_len(o) : 0; }
  public float connectionRating() { int o = __offset(6); return o != 0 ? bb.getFloat(o + bb_pos) : 0; }

  public static int createRating(FlatBufferBuilder builder,
      int rating_elements,
      float connection_rating) {
    builder.startObject(2);
    Rating.addConnectionRating(builder, connection_rating);
    Rating.addRatingElements(builder, rating_elements);
    return Rating.endRating(builder);
  }

  public static void startRating(FlatBufferBuilder builder) { builder.startObject(2); }
  public static void addRatingElements(FlatBufferBuilder builder, int ratingElementsOffset) { builder.addOffset(0, ratingElementsOffset, 0); }
  public static int createRatingElementsVector(FlatBufferBuilder builder, int[] data) { builder.startVector(4, data.length, 4); for (int i = data.length - 1; i >= 0; i--) builder.addOffset(data[i]); return builder.endVector(); }
  public static void startRatingElementsVector(FlatBufferBuilder builder, int numElems) { builder.startVector(4, numElems, 4); }
  public static void addConnectionRating(FlatBufferBuilder builder, float connectionRating) { builder.addFloat(1, connectionRating, 0); }
  public static int endRating(FlatBufferBuilder builder) {
    int o = builder.endObject();
    return o;
  }
};

