// automatically generated, do not modify

package motis.railviz;

import java.nio.*;
import java.lang.*;
import java.util.*;
import com.google.flatbuffers.*;

public final class RailVizRouteEntry extends Table {
  public static RailVizRouteEntry getRootAsRailVizRouteEntry(ByteBuffer _bb) { return getRootAsRailVizRouteEntry(_bb, new RailVizRouteEntry()); }
  public static RailVizRouteEntry getRootAsRailVizRouteEntry(ByteBuffer _bb, RailVizRouteEntry obj) { _bb.order(ByteOrder.LITTLE_ENDIAN); return (obj.__init(_bb.getInt(_bb.position()) + _bb.position(), _bb)); }
  public RailVizRouteEntry __init(int _i, ByteBuffer _bb) { bb_pos = _i; bb = _bb; return this; }

  public long departureId() { int o = __offset(4); return o != 0 ? (long)bb.getInt(o + bb_pos) & 0xFFFFFFFFL : 0; }
  public String departureStationName() { int o = __offset(6); return o != 0 ? __string(o + bb_pos) : null; }
  public ByteBuffer departureStationNameAsByteBuffer() { return __vector_as_bytebuffer(6, 1); }
  public RailVizTrain trainDeparture() { return trainDeparture(new RailVizTrain()); }
  public RailVizTrain trainDeparture(RailVizTrain obj) { int o = __offset(8); return o != 0 ? obj.__init(__indirect(o + bb_pos), bb) : null; }

  public static int createRailVizRouteEntry(FlatBufferBuilder builder,
      long departure_id,
      int departure_station_name,
      int train_departure) {
    builder.startObject(3);
    RailVizRouteEntry.addTrainDeparture(builder, train_departure);
    RailVizRouteEntry.addDepartureStationName(builder, departure_station_name);
    RailVizRouteEntry.addDepartureId(builder, departure_id);
    return RailVizRouteEntry.endRailVizRouteEntry(builder);
  }

  public static void startRailVizRouteEntry(FlatBufferBuilder builder) { builder.startObject(3); }
  public static void addDepartureId(FlatBufferBuilder builder, long departureId) { builder.addInt(0, (int)(departureId & 0xFFFFFFFFL), 0); }
  public static void addDepartureStationName(FlatBufferBuilder builder, int departureStationNameOffset) { builder.addOffset(1, departureStationNameOffset, 0); }
  public static void addTrainDeparture(FlatBufferBuilder builder, int trainDepartureOffset) { builder.addOffset(2, trainDepartureOffset, 0); }
  public static int endRailVizRouteEntry(FlatBufferBuilder builder) {
    int o = builder.endObject();
    return o;
  }
};

