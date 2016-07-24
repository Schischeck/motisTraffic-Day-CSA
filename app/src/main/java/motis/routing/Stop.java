// automatically generated, do not modify

package motis.routing;

import java.nio.*;
import java.lang.*;
import java.util.*;
import com.google.flatbuffers.*;

public final class Stop extends Table {
  public static Stop getRootAsStop(ByteBuffer _bb) { return getRootAsStop(_bb, new Stop()); }
  public static Stop getRootAsStop(ByteBuffer _bb, Stop obj) { _bb.order(ByteOrder.LITTLE_ENDIAN); return (obj.__init(_bb.getInt(_bb.position()) + _bb.position(), _bb)); }
  public Stop __init(int _i, ByteBuffer _bb) { bb_pos = _i; bb = _bb; return this; }

  public String evaNr() { int o = __offset(4); return o != 0 ? __string(o + bb_pos) : null; }
  public ByteBuffer evaNrAsByteBuffer() { return __vector_as_bytebuffer(4, 1); }
  public String name() { int o = __offset(6); return o != 0 ? __string(o + bb_pos) : null; }
  public ByteBuffer nameAsByteBuffer() { return __vector_as_bytebuffer(6, 1); }
  public EventInfo arrival() { return arrival(new EventInfo()); }
  public EventInfo arrival(EventInfo obj) { int o = __offset(8); return o != 0 ? obj.__init(__indirect(o + bb_pos), bb) : null; }
  public EventInfo departure() { return departure(new EventInfo()); }
  public EventInfo departure(EventInfo obj) { int o = __offset(10); return o != 0 ? obj.__init(__indirect(o + bb_pos), bb) : null; }
  public boolean interchange() { int o = __offset(12); return o != 0 ? 0!=bb.get(o + bb_pos) : false; }

  public static int createStop(FlatBufferBuilder builder,
      int eva_nr,
      int name,
      int arrival,
      int departure,
      boolean interchange) {
    builder.startObject(5);
    Stop.addDeparture(builder, departure);
    Stop.addArrival(builder, arrival);
    Stop.addName(builder, name);
    Stop.addEvaNr(builder, eva_nr);
    Stop.addInterchange(builder, interchange);
    return Stop.endStop(builder);
  }

  public static void startStop(FlatBufferBuilder builder) { builder.startObject(5); }
  public static void addEvaNr(FlatBufferBuilder builder, int evaNrOffset) { builder.addOffset(0, evaNrOffset, 0); }
  public static void addName(FlatBufferBuilder builder, int nameOffset) { builder.addOffset(1, nameOffset, 0); }
  public static void addArrival(FlatBufferBuilder builder, int arrivalOffset) { builder.addOffset(2, arrivalOffset, 0); }
  public static void addDeparture(FlatBufferBuilder builder, int departureOffset) { builder.addOffset(3, departureOffset, 0); }
  public static void addInterchange(FlatBufferBuilder builder, boolean interchange) { builder.addBoolean(4, interchange, false); }
  public static int endStop(FlatBufferBuilder builder) {
    int o = builder.endObject();
    return o;
  }
};

