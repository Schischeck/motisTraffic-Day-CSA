// automatically generated, do not modify

package motis.lookup;

import java.nio.*;
import java.lang.*;
import java.util.*;
import com.google.flatbuffers.*;

public final class LookupStationEventsRequest extends Table {
  public static LookupStationEventsRequest getRootAsLookupStationEventsRequest(ByteBuffer _bb) { return getRootAsLookupStationEventsRequest(_bb, new LookupStationEventsRequest()); }
  public static LookupStationEventsRequest getRootAsLookupStationEventsRequest(ByteBuffer _bb, LookupStationEventsRequest obj) { _bb.order(ByteOrder.LITTLE_ENDIAN); return (obj.__init(_bb.getInt(_bb.position()) + _bb.position(), _bb)); }
  public LookupStationEventsRequest __init(int _i, ByteBuffer _bb) { bb_pos = _i; bb = _bb; return this; }

  public String evaNr() { int o = __offset(4); return o != 0 ? __string(o + bb_pos) : null; }
  public ByteBuffer evaNrAsByteBuffer() { return __vector_as_bytebuffer(4, 1); }
  public long begin() { int o = __offset(6); return o != 0 ? bb.getLong(o + bb_pos) : 0; }
  public long end() { int o = __offset(8); return o != 0 ? bb.getLong(o + bb_pos) : 0; }

  public static int createLookupStationEventsRequest(FlatBufferBuilder builder,
      int eva_nr,
      long begin,
      long end) {
    builder.startObject(3);
    LookupStationEventsRequest.addEnd(builder, end);
    LookupStationEventsRequest.addBegin(builder, begin);
    LookupStationEventsRequest.addEvaNr(builder, eva_nr);
    return LookupStationEventsRequest.endLookupStationEventsRequest(builder);
  }

  public static void startLookupStationEventsRequest(FlatBufferBuilder builder) { builder.startObject(3); }
  public static void addEvaNr(FlatBufferBuilder builder, int evaNrOffset) { builder.addOffset(0, evaNrOffset, 0); }
  public static void addBegin(FlatBufferBuilder builder, long begin) { builder.addLong(1, begin, 0); }
  public static void addEnd(FlatBufferBuilder builder, long end) { builder.addLong(2, end, 0); }
  public static int endLookupStationEventsRequest(FlatBufferBuilder builder) {
    int o = builder.endObject();
    return o;
  }
};

