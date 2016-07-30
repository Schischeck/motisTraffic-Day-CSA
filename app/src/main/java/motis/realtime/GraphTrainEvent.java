// automatically generated, do not modify

package motis.realtime;

import java.nio.*;
import java.lang.*;
import java.util.*;
import com.google.flatbuffers.*;

/**
 * Identifies a train event as present in the schedule graph.
 */
public final class GraphTrainEvent extends Table {
  public static GraphTrainEvent getRootAsGraphTrainEvent(ByteBuffer _bb) { return getRootAsGraphTrainEvent(_bb, new GraphTrainEvent()); }
  public static GraphTrainEvent getRootAsGraphTrainEvent(ByteBuffer _bb, GraphTrainEvent obj) { _bb.order(ByteOrder.LITTLE_ENDIAN); return (obj.__init(_bb.getInt(_bb.position()) + _bb.position(), _bb)); }
  public GraphTrainEvent __init(int _i, ByteBuffer _bb) { bb_pos = _i; bb = _bb; return this; }

  public long trainNr() { int o = __offset(4); return o != 0 ? (long)bb.getInt(o + bb_pos) & 0xFFFFFFFFL : 0; }
  public long stationIndex() { int o = __offset(6); return o != 0 ? (long)bb.getInt(o + bb_pos) & 0xFFFFFFFFL : 0; }
  public boolean departure() { int o = __offset(8); return o != 0 ? 0!=bb.get(o + bb_pos) : false; }
  /**
   * The current time of the event, as in the graph.
   */
  public int realTime() { int o = __offset(10); return o != 0 ? bb.getShort(o + bb_pos) & 0xFFFF : 0; }
  public int routeId() { int o = __offset(12); return o != 0 ? bb.getInt(o + bb_pos) : 0; }

  public static int createGraphTrainEvent(FlatBufferBuilder builder,
      long train_nr,
      long station_index,
      boolean departure,
      int real_time,
      int route_id) {
    builder.startObject(5);
    GraphTrainEvent.addRouteId(builder, route_id);
    GraphTrainEvent.addStationIndex(builder, station_index);
    GraphTrainEvent.addTrainNr(builder, train_nr);
    GraphTrainEvent.addRealTime(builder, real_time);
    GraphTrainEvent.addDeparture(builder, departure);
    return GraphTrainEvent.endGraphTrainEvent(builder);
  }

  public static void startGraphTrainEvent(FlatBufferBuilder builder) { builder.startObject(5); }
  public static void addTrainNr(FlatBufferBuilder builder, long trainNr) { builder.addInt(0, (int)(trainNr & 0xFFFFFFFFL), 0); }
  public static void addStationIndex(FlatBufferBuilder builder, long stationIndex) { builder.addInt(1, (int)(stationIndex & 0xFFFFFFFFL), 0); }
  public static void addDeparture(FlatBufferBuilder builder, boolean departure) { builder.addBoolean(2, departure, false); }
  public static void addRealTime(FlatBufferBuilder builder, int realTime) { builder.addShort(3, (short)(realTime & 0xFFFF), 0); }
  public static void addRouteId(FlatBufferBuilder builder, int routeId) { builder.addInt(4, routeId, 0); }
  public static int endGraphTrainEvent(FlatBufferBuilder builder) {
    int o = builder.endObject();
    return o;
  }
};

