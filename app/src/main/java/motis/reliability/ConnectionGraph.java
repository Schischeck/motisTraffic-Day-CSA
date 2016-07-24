// automatically generated, do not modify

package motis.reliability;

import java.nio.*;
import java.lang.*;
import java.util.*;
import com.google.flatbuffers.*;

import motis.routing.Connection;

public final class ConnectionGraph extends Table {
  public static ConnectionGraph getRootAsConnectionGraph(ByteBuffer _bb) { return getRootAsConnectionGraph(_bb, new ConnectionGraph()); }
  public static ConnectionGraph getRootAsConnectionGraph(ByteBuffer _bb, ConnectionGraph obj) { _bb.order(ByteOrder.LITTLE_ENDIAN); return (obj.__init(_bb.getInt(_bb.position()) + _bb.position(), _bb)); }
  public ConnectionGraph __init(int _i, ByteBuffer _bb) { bb_pos = _i; bb = _bb; return this; }

  public Stop stops(int j) { return stops(new Stop(), j); }
  public Stop stops(Stop obj, int j) { int o = __offset(4); return o != 0 ? obj.__init(__indirect(__vector(o) + j * 4), bb) : null; }
  public int stopsLength() { int o = __offset(4); return o != 0 ? __vector_len(o) : 0; }
  public Connection journeys(int j) { return journeys(new Connection(), j); }
  public Connection journeys(Connection obj, int j) { int o = __offset(6); return o != 0 ? obj.__init(__indirect(__vector(o) + j * 4), bb) : null; }
  public int journeysLength() { int o = __offset(6); return o != 0 ? __vector_len(o) : 0; }
  public ProbabilityDistribution arrivalDistribution() { return arrivalDistribution(new ProbabilityDistribution()); }
  public ProbabilityDistribution arrivalDistribution(ProbabilityDistribution obj) { int o = __offset(8); return o != 0 ? obj.__init(__indirect(o + bb_pos), bb) : null; }

  public static int createConnectionGraph(FlatBufferBuilder builder,
      int stops,
      int journeys,
      int arrival_distribution) {
    builder.startObject(3);
    ConnectionGraph.addArrivalDistribution(builder, arrival_distribution);
    ConnectionGraph.addJourneys(builder, journeys);
    ConnectionGraph.addStops(builder, stops);
    return ConnectionGraph.endConnectionGraph(builder);
  }

  public static void startConnectionGraph(FlatBufferBuilder builder) { builder.startObject(3); }
  public static void addStops(FlatBufferBuilder builder, int stopsOffset) { builder.addOffset(0, stopsOffset, 0); }
  public static int createStopsVector(FlatBufferBuilder builder, int[] data) { builder.startVector(4, data.length, 4); for (int i = data.length - 1; i >= 0; i--) builder.addOffset(data[i]); return builder.endVector(); }
  public static void startStopsVector(FlatBufferBuilder builder, int numElems) { builder.startVector(4, numElems, 4); }
  public static void addJourneys(FlatBufferBuilder builder, int journeysOffset) { builder.addOffset(1, journeysOffset, 0); }
  public static int createJourneysVector(FlatBufferBuilder builder, int[] data) { builder.startVector(4, data.length, 4); for (int i = data.length - 1; i >= 0; i--) builder.addOffset(data[i]); return builder.endVector(); }
  public static void startJourneysVector(FlatBufferBuilder builder, int numElems) { builder.startVector(4, numElems, 4); }
  public static void addArrivalDistribution(FlatBufferBuilder builder, int arrivalDistributionOffset) { builder.addOffset(2, arrivalDistributionOffset, 0); }
  public static int endConnectionGraph(FlatBufferBuilder builder) {
    int o = builder.endObject();
    return o;
  }
};

