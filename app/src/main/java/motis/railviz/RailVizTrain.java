// automatically generated, do not modify

package motis.railviz;

import java.nio.*;
import java.lang.*;
import java.util.*;
import com.google.flatbuffers.*;

public final class RailVizTrain extends Table {
  public static RailVizTrain getRootAsRailVizTrain(ByteBuffer _bb) { return getRootAsRailVizTrain(_bb, new RailVizTrain()); }
  public static RailVizTrain getRootAsRailVizTrain(ByteBuffer _bb, RailVizTrain obj) { _bb.order(ByteOrder.LITTLE_ENDIAN); return (obj.__init(_bb.getInt(_bb.position()) + _bb.position(), _bb)); }
  public RailVizTrain __init(int _i, ByteBuffer _bb) { bb_pos = _i; bb = _bb; return this; }

  public int dTime() { int o = __offset(4); return o != 0 ? bb.getInt(o + bb_pos) : 0; }
  public int aTime() { int o = __offset(6); return o != 0 ? bb.getInt(o + bb_pos) : 0; }
  public int dStation() { int o = __offset(8); return o != 0 ? bb.getInt(o + bb_pos) : 0; }
  public int aStation() { int o = __offset(10); return o != 0 ? bb.getInt(o + bb_pos) : 0; }
  public int routeId() { int o = __offset(12); return o != 0 ? bb.getInt(o + bb_pos) : 0; }
  public int dTimeDelay() { int o = __offset(14); return o != 0 ? bb.getInt(o + bb_pos) : 0; }
  public int aTimeDelay() { int o = __offset(16); return o != 0 ? bb.getInt(o + bb_pos) : 0; }
  public String departureDelayReason() { int o = __offset(18); return o != 0 ? __string(o + bb_pos) : null; }
  public ByteBuffer departureDelayReasonAsByteBuffer() { return __vector_as_bytebuffer(18, 1); }
  public String arrivalDelayReason() { int o = __offset(20); return o != 0 ? __string(o + bb_pos) : null; }
  public ByteBuffer arrivalDelayReasonAsByteBuffer() { return __vector_as_bytebuffer(20, 1); }

  public static int createRailVizTrain(FlatBufferBuilder builder,
      int d_time,
      int a_time,
      int d_station,
      int a_station,
      int route_id,
      int d_time_delay,
      int a_time_delay,
      int departure_delay_reason,
      int arrival_delay_reason) {
    builder.startObject(9);
    RailVizTrain.addArrivalDelayReason(builder, arrival_delay_reason);
    RailVizTrain.addDepartureDelayReason(builder, departure_delay_reason);
    RailVizTrain.addATimeDelay(builder, a_time_delay);
    RailVizTrain.addDTimeDelay(builder, d_time_delay);
    RailVizTrain.addRouteId(builder, route_id);
    RailVizTrain.addAStation(builder, a_station);
    RailVizTrain.addDStation(builder, d_station);
    RailVizTrain.addATime(builder, a_time);
    RailVizTrain.addDTime(builder, d_time);
    return RailVizTrain.endRailVizTrain(builder);
  }

  public static void startRailVizTrain(FlatBufferBuilder builder) { builder.startObject(9); }
  public static void addDTime(FlatBufferBuilder builder, int dTime) { builder.addInt(0, dTime, 0); }
  public static void addATime(FlatBufferBuilder builder, int aTime) { builder.addInt(1, aTime, 0); }
  public static void addDStation(FlatBufferBuilder builder, int dStation) { builder.addInt(2, dStation, 0); }
  public static void addAStation(FlatBufferBuilder builder, int aStation) { builder.addInt(3, aStation, 0); }
  public static void addRouteId(FlatBufferBuilder builder, int routeId) { builder.addInt(4, routeId, 0); }
  public static void addDTimeDelay(FlatBufferBuilder builder, int dTimeDelay) { builder.addInt(5, dTimeDelay, 0); }
  public static void addATimeDelay(FlatBufferBuilder builder, int aTimeDelay) { builder.addInt(6, aTimeDelay, 0); }
  public static void addDepartureDelayReason(FlatBufferBuilder builder, int departureDelayReasonOffset) { builder.addOffset(7, departureDelayReasonOffset, 0); }
  public static void addArrivalDelayReason(FlatBufferBuilder builder, int arrivalDelayReasonOffset) { builder.addOffset(8, arrivalDelayReasonOffset, 0); }
  public static int endRailVizTrain(FlatBufferBuilder builder) {
    int o = builder.endObject();
    return o;
  }
};

