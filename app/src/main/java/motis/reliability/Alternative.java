// automatically generated, do not modify

package motis.reliability;

import java.nio.*;
import java.lang.*;
import java.util.*;
import com.google.flatbuffers.*;

public final class Alternative extends Table {
  public static Alternative getRootAsAlternative(ByteBuffer _bb) { return getRootAsAlternative(_bb, new Alternative()); }
  public static Alternative getRootAsAlternative(ByteBuffer _bb, Alternative obj) { _bb.order(ByteOrder.LITTLE_ENDIAN); return (obj.__init(_bb.getInt(_bb.position()) + _bb.position(), _bb)); }
  public Alternative __init(int _i, ByteBuffer _bb) { bb_pos = _i; bb = _bb; return this; }

  public short journey() { int o = __offset(4); return o != 0 ? bb.getShort(o + bb_pos) : 0; }
  public short nextStop() { int o = __offset(6); return o != 0 ? bb.getShort(o + bb_pos) : 0; }
  public AlternativeRating rating() { return rating(new AlternativeRating()); }
  public AlternativeRating rating(AlternativeRating obj) { int o = __offset(8); return o != 0 ? obj.__init(__indirect(o + bb_pos), bb) : null; }

  public static int createAlternative(FlatBufferBuilder builder,
      short journey,
      short next_stop,
      int rating) {
    builder.startObject(3);
    Alternative.addRating(builder, rating);
    Alternative.addNextStop(builder, next_stop);
    Alternative.addJourney(builder, journey);
    return Alternative.endAlternative(builder);
  }

  public static void startAlternative(FlatBufferBuilder builder) { builder.startObject(3); }
  public static void addJourney(FlatBufferBuilder builder, short journey) { builder.addShort(0, journey, 0); }
  public static void addNextStop(FlatBufferBuilder builder, short nextStop) { builder.addShort(1, nextStop, 0); }
  public static void addRating(FlatBufferBuilder builder, int ratingOffset) { builder.addOffset(2, ratingOffset, 0); }
  public static int endAlternative(FlatBufferBuilder builder) {
    int o = builder.endObject();
    return o;
  }
};

