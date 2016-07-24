// automatically generated, do not modify

package motis.reliability;

import java.nio.*;
import java.lang.*;
import java.util.*;
import com.google.flatbuffers.*;

public final class ReliableSearchReq extends Table {
  public static ReliableSearchReq getRootAsReliableSearchReq(ByteBuffer _bb) { return getRootAsReliableSearchReq(_bb, new ReliableSearchReq()); }
  public static ReliableSearchReq getRootAsReliableSearchReq(ByteBuffer _bb, ReliableSearchReq obj) { _bb.order(ByteOrder.LITTLE_ENDIAN); return (obj.__init(_bb.getInt(_bb.position()) + _bb.position(), _bb)); }
  public ReliableSearchReq __init(int _i, ByteBuffer _bb) { bb_pos = _i; bb = _bb; return this; }

  public short minDepartureDiff() { int o = __offset(4); return o != 0 ? bb.getShort(o + bb_pos) : 0; }

  public static int createReliableSearchReq(FlatBufferBuilder builder,
      short min_departure_diff) {
    builder.startObject(1);
    ReliableSearchReq.addMinDepartureDiff(builder, min_departure_diff);
    return ReliableSearchReq.endReliableSearchReq(builder);
  }

  public static void startReliableSearchReq(FlatBufferBuilder builder) { builder.startObject(1); }
  public static void addMinDepartureDiff(FlatBufferBuilder builder, short minDepartureDiff) { builder.addShort(0, minDepartureDiff, 0); }
  public static int endReliableSearchReq(FlatBufferBuilder builder) {
    int o = builder.endObject();
    return o;
  }
};

