// automatically generated, do not modify

package motis.routing;

import java.nio.*;
import java.lang.*;
import java.util.*;
import com.google.flatbuffers.*;

public final class MumoEdge extends Table {
  public static MumoEdge getRootAsMumoEdge(ByteBuffer _bb) { return getRootAsMumoEdge(_bb, new MumoEdge()); }
  public static MumoEdge getRootAsMumoEdge(ByteBuffer _bb, MumoEdge obj) { _bb.order(ByteOrder.LITTLE_ENDIAN); return (obj.__init(_bb.getInt(_bb.position()) + _bb.position(), _bb)); }
  public MumoEdge __init(int _i, ByteBuffer _bb) { bb_pos = _i; bb = _bb; return this; }

  public String fromStationEva() { int o = __offset(4); return o != 0 ? __string(o + bb_pos) : null; }
  public ByteBuffer fromStationEvaAsByteBuffer() { return __vector_as_bytebuffer(4, 1); }
  public String toStationEva() { int o = __offset(6); return o != 0 ? __string(o + bb_pos) : null; }
  public ByteBuffer toStationEvaAsByteBuffer() { return __vector_as_bytebuffer(6, 1); }
  public int duration() { int o = __offset(8); return o != 0 ? bb.getShort(o + bb_pos) & 0xFFFF : 0; }
  public int price() { int o = __offset(10); return o != 0 ? bb.getShort(o + bb_pos) & 0xFFFF : 0; }

  public static int createMumoEdge(FlatBufferBuilder builder,
      int from_station_eva,
      int to_station_eva,
      int duration,
      int price) {
    builder.startObject(4);
    MumoEdge.addToStationEva(builder, to_station_eva);
    MumoEdge.addFromStationEva(builder, from_station_eva);
    MumoEdge.addPrice(builder, price);
    MumoEdge.addDuration(builder, duration);
    return MumoEdge.endMumoEdge(builder);
  }

  public static void startMumoEdge(FlatBufferBuilder builder) { builder.startObject(4); }
  public static void addFromStationEva(FlatBufferBuilder builder, int fromStationEvaOffset) { builder.addOffset(0, fromStationEvaOffset, 0); }
  public static void addToStationEva(FlatBufferBuilder builder, int toStationEvaOffset) { builder.addOffset(1, toStationEvaOffset, 0); }
  public static void addDuration(FlatBufferBuilder builder, int duration) { builder.addShort(2, (short)(duration & 0xFFFF), 0); }
  public static void addPrice(FlatBufferBuilder builder, int price) { builder.addShort(3, (short)(price & 0xFFFF), 0); }
  public static int endMumoEdge(FlatBufferBuilder builder) {
    int o = builder.endObject();
    return o;
  }
};

