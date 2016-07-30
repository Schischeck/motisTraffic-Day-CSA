// automatically generated, do not modify

package motis.lookup;

import java.nio.*;
import java.lang.*;
import java.util.*;
import com.google.flatbuffers.*;

public final class IdEvent extends Table {
  public static IdEvent getRootAsIdEvent(ByteBuffer _bb) { return getRootAsIdEvent(_bb, new IdEvent()); }
  public static IdEvent getRootAsIdEvent(ByteBuffer _bb, IdEvent obj) { _bb.order(ByteOrder.LITTLE_ENDIAN); return (obj.__init(_bb.getInt(_bb.position()) + _bb.position(), _bb)); }
  public IdEvent __init(int _i, ByteBuffer _bb) { bb_pos = _i; bb = _bb; return this; }

  public String evaNr() { int o = __offset(4); return o != 0 ? __string(o + bb_pos) : null; }
  public ByteBuffer evaNrAsByteBuffer() { return __vector_as_bytebuffer(4, 1); }
  public boolean type() { int o = __offset(6); return o != 0 ? 0!=bb.get(o + bb_pos) : false; }
  public long trainNr() { int o = __offset(8); return o != 0 ? (long)bb.getInt(o + bb_pos) & 0xFFFFFFFFL : 0; }
  public String lineId() { int o = __offset(10); return o != 0 ? __string(o + bb_pos) : null; }
  public ByteBuffer lineIdAsByteBuffer() { return __vector_as_bytebuffer(10, 1); }
  public long scheduleTime() { int o = __offset(12); return o != 0 ? bb.getLong(o + bb_pos) : 0; }

  public static int createIdEvent(FlatBufferBuilder builder,
      int eva_nr,
      boolean type,
      long train_nr,
      int line_id,
      long schedule_time) {
    builder.startObject(5);
    IdEvent.addScheduleTime(builder, schedule_time);
    IdEvent.addLineId(builder, line_id);
    IdEvent.addTrainNr(builder, train_nr);
    IdEvent.addEvaNr(builder, eva_nr);
    IdEvent.addType(builder, type);
    return IdEvent.endIdEvent(builder);
  }

  public static void startIdEvent(FlatBufferBuilder builder) { builder.startObject(5); }
  public static void addEvaNr(FlatBufferBuilder builder, int evaNrOffset) { builder.addOffset(0, evaNrOffset, 0); }
  public static void addType(FlatBufferBuilder builder, boolean type) { builder.addBoolean(1, type, false); }
  public static void addTrainNr(FlatBufferBuilder builder, long trainNr) { builder.addInt(2, (int)(trainNr & 0xFFFFFFFFL), 0); }
  public static void addLineId(FlatBufferBuilder builder, int lineIdOffset) { builder.addOffset(3, lineIdOffset, 0); }
  public static void addScheduleTime(FlatBufferBuilder builder, long scheduleTime) { builder.addLong(4, scheduleTime, 0); }
  public static int endIdEvent(FlatBufferBuilder builder) {
    int o = builder.endObject();
    return o;
  }
};

