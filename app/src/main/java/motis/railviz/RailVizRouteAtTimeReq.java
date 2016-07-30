// automatically generated, do not modify

package motis.railviz;

import java.nio.*;
import java.lang.*;
import java.util.*;
import com.google.flatbuffers.*;

public final class RailVizRouteAtTimeReq extends Table {
  public static RailVizRouteAtTimeReq getRootAsRailVizRouteAtTimeReq(ByteBuffer _bb) { return getRootAsRailVizRouteAtTimeReq(_bb, new RailVizRouteAtTimeReq()); }
  public static RailVizRouteAtTimeReq getRootAsRailVizRouteAtTimeReq(ByteBuffer _bb, RailVizRouteAtTimeReq obj) { _bb.order(ByteOrder.LITTLE_ENDIAN); return (obj.__init(_bb.getInt(_bb.position()) + _bb.position(), _bb)); }
  public RailVizRouteAtTimeReq __init(int _i, ByteBuffer _bb) { bb_pos = _i; bb = _bb; return this; }

  public long stationId() { int o = __offset(4); return o != 0 ? (long)bb.getInt(o + bb_pos) & 0xFFFFFFFFL : 0; }
  public long departureTime() { int o = __offset(6); return o != 0 ? (long)bb.getInt(o + bb_pos) & 0xFFFFFFFFL : 0; }
  public long routeId() { int o = __offset(8); return o != 0 ? (long)bb.getInt(o + bb_pos) & 0xFFFFFFFFL : 0; }

  public static int createRailVizRouteAtTimeReq(FlatBufferBuilder builder,
      long station_id,
      long departure_time,
      long route_id) {
    builder.startObject(3);
    RailVizRouteAtTimeReq.addRouteId(builder, route_id);
    RailVizRouteAtTimeReq.addDepartureTime(builder, departure_time);
    RailVizRouteAtTimeReq.addStationId(builder, station_id);
    return RailVizRouteAtTimeReq.endRailVizRouteAtTimeReq(builder);
  }

  public static void startRailVizRouteAtTimeReq(FlatBufferBuilder builder) { builder.startObject(3); }
  public static void addStationId(FlatBufferBuilder builder, long stationId) { builder.addInt(0, (int)(stationId & 0xFFFFFFFFL), 0); }
  public static void addDepartureTime(FlatBufferBuilder builder, long departureTime) { builder.addInt(1, (int)(departureTime & 0xFFFFFFFFL), 0); }
  public static void addRouteId(FlatBufferBuilder builder, long routeId) { builder.addInt(2, (int)(routeId & 0xFFFFFFFFL), 0); }
  public static int endRailVizRouteAtTimeReq(FlatBufferBuilder builder) {
    int o = builder.endObject();
    return o;
  }
};

