// automatically generated, do not modify

package motis.routing;

import java.nio.*;
import java.lang.*;
import java.util.*;
import com.google.flatbuffers.*;

public final class HotelEdge extends Table {
  public static HotelEdge getRootAsHotelEdge(ByteBuffer _bb) { return getRootAsHotelEdge(_bb, new HotelEdge()); }
  public static HotelEdge getRootAsHotelEdge(ByteBuffer _bb, HotelEdge obj) { _bb.order(ByteOrder.LITTLE_ENDIAN); return (obj.__init(_bb.getInt(_bb.position()) + _bb.position(), _bb)); }
  public HotelEdge __init(int _i, ByteBuffer _bb) { bb_pos = _i; bb = _bb; return this; }

  public String stationEva() { int o = __offset(4); return o != 0 ? __string(o + bb_pos) : null; }
  public ByteBuffer stationEvaAsByteBuffer() { return __vector_as_bytebuffer(4, 1); }
  public int earliestCheckoutTime() { int o = __offset(6); return o != 0 ? bb.getShort(o + bb_pos) & 0xFFFF : 0; }
  public int minStayDuration() { int o = __offset(8); return o != 0 ? bb.getShort(o + bb_pos) & 0xFFFF : 0; }
  public int price() { int o = __offset(10); return o != 0 ? bb.getShort(o + bb_pos) & 0xFFFF : 0; }

  public static int createHotelEdge(FlatBufferBuilder builder,
      int station_eva,
      int earliest_checkout_time,
      int min_stay_duration,
      int price) {
    builder.startObject(4);
    HotelEdge.addStationEva(builder, station_eva);
    HotelEdge.addPrice(builder, price);
    HotelEdge.addMinStayDuration(builder, min_stay_duration);
    HotelEdge.addEarliestCheckoutTime(builder, earliest_checkout_time);
    return HotelEdge.endHotelEdge(builder);
  }

  public static void startHotelEdge(FlatBufferBuilder builder) { builder.startObject(4); }
  public static void addStationEva(FlatBufferBuilder builder, int stationEvaOffset) { builder.addOffset(0, stationEvaOffset, 0); }
  public static void addEarliestCheckoutTime(FlatBufferBuilder builder, int earliestCheckoutTime) { builder.addShort(1, (short)(earliestCheckoutTime & 0xFFFF), 0); }
  public static void addMinStayDuration(FlatBufferBuilder builder, int minStayDuration) { builder.addShort(2, (short)(minStayDuration & 0xFFFF), 0); }
  public static void addPrice(FlatBufferBuilder builder, int price) { builder.addShort(3, (short)(price & 0xFFFF), 0); }
  public static int endHotelEdge(FlatBufferBuilder builder) {
    int o = builder.endObject();
    return o;
  }
};

