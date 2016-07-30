// automatically generated, do not modify

package motis.reliability;

import java.nio.*;
import java.lang.*;
import java.util.*;
import com.google.flatbuffers.*;

public final class SimpleRatingInfo extends Table {
  public static SimpleRatingInfo getRootAsSimpleRatingInfo(ByteBuffer _bb) { return getRootAsSimpleRatingInfo(_bb, new SimpleRatingInfo()); }
  public static SimpleRatingInfo getRootAsSimpleRatingInfo(ByteBuffer _bb, SimpleRatingInfo obj) { _bb.order(ByteOrder.LITTLE_ENDIAN); return (obj.__init(_bb.getInt(_bb.position()) + _bb.position(), _bb)); }
  public SimpleRatingInfo __init(int _i, ByteBuffer _bb) { bb_pos = _i; bb = _bb; return this; }

  public String type() { int o = __offset(4); return o != 0 ? __string(o + bb_pos) : null; }
  public ByteBuffer typeAsByteBuffer() { return __vector_as_bytebuffer(4, 1); }
  public float rating() { int o = __offset(6); return o != 0 ? bb.getFloat(o + bb_pos) : 0; }

  public static int createSimpleRatingInfo(FlatBufferBuilder builder,
      int type,
      float rating) {
    builder.startObject(2);
    SimpleRatingInfo.addRating(builder, rating);
    SimpleRatingInfo.addType(builder, type);
    return SimpleRatingInfo.endSimpleRatingInfo(builder);
  }

  public static void startSimpleRatingInfo(FlatBufferBuilder builder) { builder.startObject(2); }
  public static void addType(FlatBufferBuilder builder, int typeOffset) { builder.addOffset(0, typeOffset, 0); }
  public static void addRating(FlatBufferBuilder builder, float rating) { builder.addFloat(1, rating, 0); }
  public static int endSimpleRatingInfo(FlatBufferBuilder builder) {
    int o = builder.endObject();
    return o;
  }
};

