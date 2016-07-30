// automatically generated, do not modify

package motis.reliability;

import java.nio.*;
import java.lang.*;
import java.util.*;
import com.google.flatbuffers.*;

public final class RatingReq extends Table {
  public static RatingReq getRootAsRatingReq(ByteBuffer _bb) { return getRootAsRatingReq(_bb, new RatingReq()); }
  public static RatingReq getRootAsRatingReq(ByteBuffer _bb, RatingReq obj) { _bb.order(ByteOrder.LITTLE_ENDIAN); return (obj.__init(_bb.getInt(_bb.position()) + _bb.position(), _bb)); }
  public RatingReq __init(int _i, ByteBuffer _bb) { bb_pos = _i; bb = _bb; return this; }


  public static void startRatingReq(FlatBufferBuilder builder) { builder.startObject(0); }
  public static int endRatingReq(FlatBufferBuilder builder) {
    int o = builder.endObject();
    return o;
  }
};

