// automatically generated, do not modify

package motis.realtime;

import java.nio.*;
import java.lang.*;
import java.util.*;
import com.google.flatbuffers.*;

public final class EventInfo extends Table {
  public static EventInfo getRootAsEventInfo(ByteBuffer _bb) { return getRootAsEventInfo(_bb, new EventInfo()); }
  public static EventInfo getRootAsEventInfo(ByteBuffer _bb, EventInfo obj) { _bb.order(ByteOrder.LITTLE_ENDIAN); return (obj.__init(_bb.getInt(_bb.position()) + _bb.position(), _bb)); }
  public EventInfo __init(int _i, ByteBuffer _bb) { bb_pos = _i; bb = _bb; return this; }

  public long trainNr() { int o = __offset(4); return o != 0 ? (long)bb.getInt(o + bb_pos) & 0xFFFFFFFFL : 0; }
  public long stationIndex() { int o = __offset(6); return o != 0 ? (long)bb.getInt(o + bb_pos) & 0xFFFFFFFFL : 0; }
  public boolean departure() { int o = __offset(8); return o != 0 ? 0!=bb.get(o + bb_pos) : false; }
  /**
   * Current time of the event (as in the graph, i.e. the delayed time)
   */
  public int realTime() { int o = __offset(10); return o != 0 ? bb.getShort(o + bb_pos) & 0xFFFF : 0; }
  /**
   * Scheduled time of the event (as in the original schedule)
   */
  public int scheduledTime() { int o = __offset(12); return o != 0 ? bb.getShort(o + bb_pos) & 0xFFFF : 0; }
  /**
   * Reason for the timestamp/delay
   */
  public byte reason() { int o = __offset(14); return o != 0 ? bb.get(o + bb_pos) : 0; }

  public static int createEventInfo(FlatBufferBuilder builder,
      long train_nr,
      long station_index,
      boolean departure,
      int real_time,
      int scheduled_time,
      byte reason) {
    builder.startObject(6);
    EventInfo.addStationIndex(builder, station_index);
    EventInfo.addTrainNr(builder, train_nr);
    EventInfo.addScheduledTime(builder, scheduled_time);
    EventInfo.addRealTime(builder, real_time);
    EventInfo.addReason(builder, reason);
    EventInfo.addDeparture(builder, departure);
    return EventInfo.endEventInfo(builder);
  }

  public static void startEventInfo(FlatBufferBuilder builder) { builder.startObject(6); }
  public static void addTrainNr(FlatBufferBuilder builder, long trainNr) { builder.addInt(0, (int)(trainNr & 0xFFFFFFFFL), 0); }
  public static void addStationIndex(FlatBufferBuilder builder, long stationIndex) { builder.addInt(1, (int)(stationIndex & 0xFFFFFFFFL), 0); }
  public static void addDeparture(FlatBufferBuilder builder, boolean departure) { builder.addBoolean(2, departure, false); }
  public static void addRealTime(FlatBufferBuilder builder, int realTime) { builder.addShort(3, (short)(realTime & 0xFFFF), 0); }
  public static void addScheduledTime(FlatBufferBuilder builder, int scheduledTime) { builder.addShort(4, (short)(scheduledTime & 0xFFFF), 0); }
  public static void addReason(FlatBufferBuilder builder, byte reason) { builder.addByte(5, reason, 0); }
  public static int endEventInfo(FlatBufferBuilder builder) {
    int o = builder.endObject();
    return o;
  }
};

