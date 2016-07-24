// automatically generated, do not modify

package motis.reliability;

import java.nio.*;
import java.lang.*;
import java.util.*;
import com.google.flatbuffers.*;

import motis.routing.RoutingResponse;

public final class ReliabilityRatingResponse extends Table {
  public static ReliabilityRatingResponse getRootAsReliabilityRatingResponse(ByteBuffer _bb) { return getRootAsReliabilityRatingResponse(_bb, new ReliabilityRatingResponse()); }
  public static ReliabilityRatingResponse getRootAsReliabilityRatingResponse(ByteBuffer _bb, ReliabilityRatingResponse obj) { _bb.order(ByteOrder.LITTLE_ENDIAN); return (obj.__init(_bb.getInt(_bb.position()) + _bb.position(), _bb)); }
  public ReliabilityRatingResponse __init(int _i, ByteBuffer _bb) { bb_pos = _i; bb = _bb; return this; }

  public RoutingResponse response() { return response(new RoutingResponse()); }
  public RoutingResponse response(RoutingResponse obj) { int o = __offset(4); return o != 0 ? obj.__init(__indirect(o + bb_pos), bb) : null; }
  public Rating ratings(int j) { return ratings(new Rating(), j); }
  public Rating ratings(Rating obj, int j) { int o = __offset(6); return o != 0 ? obj.__init(__indirect(__vector(o) + j * 4), bb) : null; }
  public int ratingsLength() { int o = __offset(6); return o != 0 ? __vector_len(o) : 0; }
  public SimpleRating simpleRatings(int j) { return simpleRatings(new SimpleRating(), j); }
  public SimpleRating simpleRatings(SimpleRating obj, int j) { int o = __offset(8); return o != 0 ? obj.__init(__indirect(__vector(o) + j * 4), bb) : null; }
  public int simpleRatingsLength() { int o = __offset(8); return o != 0 ? __vector_len(o) : 0; }

  public static int createReliabilityRatingResponse(FlatBufferBuilder builder,
      int response,
      int ratings,
      int simple_ratings) {
    builder.startObject(3);
    ReliabilityRatingResponse.addSimpleRatings(builder, simple_ratings);
    ReliabilityRatingResponse.addRatings(builder, ratings);
    ReliabilityRatingResponse.addResponse(builder, response);
    return ReliabilityRatingResponse.endReliabilityRatingResponse(builder);
  }

  public static void startReliabilityRatingResponse(FlatBufferBuilder builder) { builder.startObject(3); }
  public static void addResponse(FlatBufferBuilder builder, int responseOffset) { builder.addOffset(0, responseOffset, 0); }
  public static void addRatings(FlatBufferBuilder builder, int ratingsOffset) { builder.addOffset(1, ratingsOffset, 0); }
  public static int createRatingsVector(FlatBufferBuilder builder, int[] data) { builder.startVector(4, data.length, 4); for (int i = data.length - 1; i >= 0; i--) builder.addOffset(data[i]); return builder.endVector(); }
  public static void startRatingsVector(FlatBufferBuilder builder, int numElems) { builder.startVector(4, numElems, 4); }
  public static void addSimpleRatings(FlatBufferBuilder builder, int simpleRatingsOffset) { builder.addOffset(2, simpleRatingsOffset, 0); }
  public static int createSimpleRatingsVector(FlatBufferBuilder builder, int[] data) { builder.startVector(4, data.length, 4); for (int i = data.length - 1; i >= 0; i--) builder.addOffset(data[i]); return builder.endVector(); }
  public static void startSimpleRatingsVector(FlatBufferBuilder builder, int numElems) { builder.startVector(4, numElems, 4); }
  public static int endReliabilityRatingResponse(FlatBufferBuilder builder) {
    int o = builder.endObject();
    return o;
  }
};

