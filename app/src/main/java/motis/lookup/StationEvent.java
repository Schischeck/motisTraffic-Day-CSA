// automatically generated, do not modify

package motis.lookup;

import java.nio.*;
import java.lang.*;
import java.util.*;
import com.google.flatbuffers.*;

public final class StationEvent extends Table {
  public static StationEvent getRootAsStationEvent(ByteBuffer _bb) { return getRootAsStationEvent(_bb, new StationEvent()); }
  public static StationEvent getRootAsStationEvent(ByteBuffer _bb, StationEvent obj) { _bb.order(ByteOrder.LITTLE_ENDIAN); return (obj.__init(_bb.getInt(_bb.position()) + _bb.position(), _bb)); }
  public StationEvent __init(int _i, ByteBuffer _bb) { bb_pos = _i; bb = _bb; return this; }

  public IdEvent idEvent() { return idEvent(new IdEvent()); }
  public IdEvent idEvent(IdEvent obj) { int o = __offset(4); return o != 0 ? obj.__init(__indirect(o + bb_pos), bb) : null; }
  public boolean type() { int o = __offset(6); return o != 0 ? 0!=bb.get(o + bb_pos) : false; }
  public long trainNr() { int o = __offset(8); return o != 0 ? (long)bb.getInt(o + bb_pos) & 0xFFFFFFFFL : 0; }
  public String lineId() { int o = __offset(10); return o != 0 ? __string(o + bb_pos) : null; }
  public ByteBuffer lineIdAsByteBuffer() { return __vector_as_bytebuffer(10, 1); }
  public long time() { int o = __offset(12); return o != 0 ? bb.getLong(o + bb_pos) : 0; }
  public long scheduleTime() { int o = __offset(14); return o != 0 ? bb.getLong(o + bb_pos) : 0; }

  public static int createStationEvent(FlatBufferBuilder builder,
      int id_event,
      boolean type,
      long train_nr,
      int line_id,
      long time,
      long schedule_time) {
    builder.startObject(6);
    StationEvent.addScheduleTime(builder, schedule_time);
    StationEvent.addTime(builder, time);
    StationEvent.addLineId(builder, line_id);
    StationEvent.addTrainNr(builder, train_nr);
    StationEvent.addIdEvent(builder, id_event);
    StationEvent.addType(builder, type);
    return StationEvent.endStationEvent(builder);
  }

  public static void startStationEvent(FlatBufferBuilder builder) { builder.startObject(6); }
  public static void addIdEvent(FlatBufferBuilder builder, int idEventOffset) { builder.addOffset(0, idEventOffset, 0); }
  public static void addType(FlatBufferBuilder builder, boolean type) { builder.addBoolean(1, type, false); }
  public static void addTrainNr(FlatBufferBuilder builder, long trainNr) { builder.addInt(2, (int)(trainNr & 0xFFFFFFFFL), 0); }
  public static void addLineId(FlatBufferBuilder builder, int lineIdOffset) { builder.addOffset(3, lineIdOffset, 0); }
  public static void addTime(FlatBufferBuilder builder, long time) { builder.addLong(4, time, 0); }
  public static void addScheduleTime(FlatBufferBuilder builder, long scheduleTime) { builder.addLong(5, scheduleTime, 0); }
  public static int endStationEvent(FlatBufferBuilder builder) {
    int o = builder.endObject();
    return o;
  }
};

