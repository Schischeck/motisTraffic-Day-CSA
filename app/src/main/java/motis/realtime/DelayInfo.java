// automatically generated, do not modify

package motis.realtime;

import java.nio.*;
import java.lang.*;
import java.util.*;
import com.google.flatbuffers.*;

public final class DelayInfo extends Table {
  public static DelayInfo getRootAsDelayInfo(ByteBuffer _bb) { return getRootAsDelayInfo(_bb, new DelayInfo()); }
  public static DelayInfo getRootAsDelayInfo(ByteBuffer _bb, DelayInfo obj) { _bb.order(ByteOrder.LITTLE_ENDIAN); return (obj.__init(_bb.getInt(_bb.position()) + _bb.position(), _bb)); }
  public DelayInfo __init(int _i, ByteBuffer _bb) { bb_pos = _i; bb = _bb; return this; }

  public long trainNr() { int o = __offset(4); return o != 0 ? (long)bb.getInt(o + bb_pos) & 0xFFFFFFFFL : 0; }
  public long stationIndex() { int o = __offset(6); return o != 0 ? (long)bb.getInt(o + bb_pos) & 0xFFFFFFFFL : 0; }
  public boolean departure() { int o = __offset(8); return o != 0 ? 0!=bb.get(o + bb_pos) : false; }
  public int scheduledTime() { int o = __offset(10); return o != 0 ? bb.getShort(o + bb_pos) & 0xFFFF : 0; }
  public int currentTime() { int o = __offset(12); return o != 0 ? bb.getShort(o + bb_pos) & 0xFFFF : 0; }
  public int forecastTime() { int o = __offset(14); return o != 0 ? bb.getShort(o + bb_pos) & 0xFFFF : 0; }
  public boolean canceled() { int o = __offset(16); return o != 0 ? 0!=bb.get(o + bb_pos) : false; }
  public byte reason() { int o = __offset(18); return o != 0 ? bb.get(o + bb_pos) : 0; }
  public int routeId() { int o = __offset(20); return o != 0 ? bb.getInt(o + bb_pos) : 0; }

  public static int createDelayInfo(FlatBufferBuilder builder,
      long train_nr,
      long station_index,
      boolean departure,
      int scheduled_time,
      int current_time,
      int forecast_time,
      boolean canceled,
      byte reason,
      int route_id) {
    builder.startObject(9);
    DelayInfo.addRouteId(builder, route_id);
    DelayInfo.addStationIndex(builder, station_index);
    DelayInfo.addTrainNr(builder, train_nr);
    DelayInfo.addForecastTime(builder, forecast_time);
    DelayInfo.addCurrentTime(builder, current_time);
    DelayInfo.addScheduledTime(builder, scheduled_time);
    DelayInfo.addReason(builder, reason);
    DelayInfo.addCanceled(builder, canceled);
    DelayInfo.addDeparture(builder, departure);
    return DelayInfo.endDelayInfo(builder);
  }

  public static void startDelayInfo(FlatBufferBuilder builder) { builder.startObject(9); }
  public static void addTrainNr(FlatBufferBuilder builder, long trainNr) { builder.addInt(0, (int)(trainNr & 0xFFFFFFFFL), 0); }
  public static void addStationIndex(FlatBufferBuilder builder, long stationIndex) { builder.addInt(1, (int)(stationIndex & 0xFFFFFFFFL), 0); }
  public static void addDeparture(FlatBufferBuilder builder, boolean departure) { builder.addBoolean(2, departure, false); }
  public static void addScheduledTime(FlatBufferBuilder builder, int scheduledTime) { builder.addShort(3, (short)(scheduledTime & 0xFFFF), 0); }
  public static void addCurrentTime(FlatBufferBuilder builder, int currentTime) { builder.addShort(4, (short)(currentTime & 0xFFFF), 0); }
  public static void addForecastTime(FlatBufferBuilder builder, int forecastTime) { builder.addShort(5, (short)(forecastTime & 0xFFFF), 0); }
  public static void addCanceled(FlatBufferBuilder builder, boolean canceled) { builder.addBoolean(6, canceled, false); }
  public static void addReason(FlatBufferBuilder builder, byte reason) { builder.addByte(7, reason, 0); }
  public static void addRouteId(FlatBufferBuilder builder, int routeId) { builder.addInt(8, routeId, 0); }
  public static int endDelayInfo(FlatBufferBuilder builder) {
    int o = builder.endObject();
    return o;
  }
};

