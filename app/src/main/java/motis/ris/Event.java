// automatically generated, do not modify

package motis.ris;

import java.nio.*;
import java.lang.*;
import java.util.*;
import com.google.flatbuffers.*;

public final class Event extends Table {
  public static Event getRootAsEvent(ByteBuffer _bb) { return getRootAsEvent(_bb, new Event()); }
  public static Event getRootAsEvent(ByteBuffer _bb, Event obj) { _bb.order(ByteOrder.LITTLE_ENDIAN); return (obj.__init(_bb.getInt(_bb.position()) + _bb.position(), _bb)); }
  public Event __init(int _i, ByteBuffer _bb) { bb_pos = _i; bb = _bb; return this; }

  public byte stationIdType() { int o = __offset(4); return o != 0 ? bb.get(o + bb_pos) : 0; }
  public String stationId() { int o = __offset(6); return o != 0 ? __string(o + bb_pos) : null; }
  public ByteBuffer stationIdAsByteBuffer() { return __vector_as_bytebuffer(6, 1); }
  public long trainIndex() { int o = __offset(8); return o != 0 ? (long)bb.getInt(o + bb_pos) & 0xFFFFFFFFL : 0; }
  public boolean type() { int o = __offset(10); return o != 0 ? 0!=bb.get(o + bb_pos) : false; }
  public long scheduledTime() { int o = __offset(12); return o != 0 ? bb.getLong(o + bb_pos) : 0; }

  public static int createEvent(FlatBufferBuilder builder,
      byte stationIdType,
      int stationId,
      long trainIndex,
      boolean type,
      long scheduledTime) {
    builder.startObject(5);
    Event.addScheduledTime(builder, scheduledTime);
    Event.addTrainIndex(builder, trainIndex);
    Event.addStationId(builder, stationId);
    Event.addType(builder, type);
    Event.addStationIdType(builder, stationIdType);
    return Event.endEvent(builder);
  }

  public static void startEvent(FlatBufferBuilder builder) { builder.startObject(5); }
  public static void addStationIdType(FlatBufferBuilder builder, byte stationIdType) { builder.addByte(0, stationIdType, 0); }
  public static void addStationId(FlatBufferBuilder builder, int stationIdOffset) { builder.addOffset(1, stationIdOffset, 0); }
  public static void addTrainIndex(FlatBufferBuilder builder, long trainIndex) { builder.addInt(2, (int)(trainIndex & 0xFFFFFFFFL), 0); }
  public static void addType(FlatBufferBuilder builder, boolean type) { builder.addBoolean(3, type, false); }
  public static void addScheduledTime(FlatBufferBuilder builder, long scheduledTime) { builder.addLong(4, scheduledTime, 0); }
  public static int endEvent(FlatBufferBuilder builder) {
    int o = builder.endObject();
    return o;
  }
};

