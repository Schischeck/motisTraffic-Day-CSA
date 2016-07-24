// automatically generated, do not modify

package motis.routing;

import java.nio.*;
import java.lang.*;
import java.util.*;
import com.google.flatbuffers.*;

public final class EventInfo extends Table {
  public static EventInfo getRootAsEventInfo(ByteBuffer _bb) { return getRootAsEventInfo(_bb, new EventInfo()); }
  public static EventInfo getRootAsEventInfo(ByteBuffer _bb, EventInfo obj) { _bb.order(ByteOrder.LITTLE_ENDIAN); return (obj.__init(_bb.getInt(_bb.position()) + _bb.position(), _bb)); }
  public EventInfo __init(int _i, ByteBuffer _bb) { bb_pos = _i; bb = _bb; return this; }

  public long time() { int o = __offset(4); return o != 0 ? bb.getLong(o + bb_pos) : 0; }
  public long scheduleTime() { int o = __offset(6); return o != 0 ? bb.getLong(o + bb_pos) : 0; }
  public String platform() { int o = __offset(8); return o != 0 ? __string(o + bb_pos) : null; }
  public ByteBuffer platformAsByteBuffer() { return __vector_as_bytebuffer(8, 1); }

  public static int createEventInfo(FlatBufferBuilder builder,
      long time,
      long schedule_time,
      int platform) {
    builder.startObject(3);
    EventInfo.addScheduleTime(builder, schedule_time);
    EventInfo.addTime(builder, time);
    EventInfo.addPlatform(builder, platform);
    return EventInfo.endEventInfo(builder);
  }

  public static void startEventInfo(FlatBufferBuilder builder) { builder.startObject(3); }
  public static void addTime(FlatBufferBuilder builder, long time) { builder.addLong(0, time, 0); }
  public static void addScheduleTime(FlatBufferBuilder builder, long scheduleTime) { builder.addLong(1, scheduleTime, 0); }
  public static void addPlatform(FlatBufferBuilder builder, int platformOffset) { builder.addOffset(2, platformOffset, 0); }
  public static int endEventInfo(FlatBufferBuilder builder) {
    int o = builder.endObject();
    return o;
  }
};

