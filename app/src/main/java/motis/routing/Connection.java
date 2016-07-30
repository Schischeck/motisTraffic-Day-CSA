// automatically generated, do not modify

package motis.routing;

import java.nio.*;
import java.lang.*;
import java.util.*;
import com.google.flatbuffers.*;

public final class Connection extends Table {
  public static Connection getRootAsConnection(ByteBuffer _bb) { return getRootAsConnection(_bb, new Connection()); }
  public static Connection getRootAsConnection(ByteBuffer _bb, Connection obj) { _bb.order(ByteOrder.LITTLE_ENDIAN); return (obj.__init(_bb.getInt(_bb.position()) + _bb.position(), _bb)); }
  public Connection __init(int _i, ByteBuffer _bb) { bb_pos = _i; bb = _bb; return this; }

  public Stop stops(int j) { return stops(new Stop(), j); }
  public Stop stops(Stop obj, int j) { int o = __offset(4); return o != 0 ? obj.__init(__indirect(__vector(o) + j * 4), bb) : null; }
  public int stopsLength() { int o = __offset(4); return o != 0 ? __vector_len(o) : 0; }
  public MoveWrapper transports(int j) { return transports(new MoveWrapper(), j); }
  public MoveWrapper transports(MoveWrapper obj, int j) { int o = __offset(6); return o != 0 ? obj.__init(__indirect(__vector(o) + j * 4), bb) : null; }
  public int transportsLength() { int o = __offset(6); return o != 0 ? __vector_len(o) : 0; }
  public Attribute attributes(int j) { return attributes(new Attribute(), j); }
  public Attribute attributes(Attribute obj, int j) { int o = __offset(8); return o != 0 ? obj.__init(__indirect(__vector(o) + j * 4), bb) : null; }
  public int attributesLength() { int o = __offset(8); return o != 0 ? __vector_len(o) : 0; }
  public int nightPenalty() { int o = __offset(10); return o != 0 ? bb.getShort(o + bb_pos) & 0xFFFF : 0; }

  public static int createConnection(FlatBufferBuilder builder,
      int stops,
      int transports,
      int attributes,
      int night_penalty) {
    builder.startObject(4);
    Connection.addAttributes(builder, attributes);
    Connection.addTransports(builder, transports);
    Connection.addStops(builder, stops);
    Connection.addNightPenalty(builder, night_penalty);
    return Connection.endConnection(builder);
  }

  public static void startConnection(FlatBufferBuilder builder) { builder.startObject(4); }
  public static void addStops(FlatBufferBuilder builder, int stopsOffset) { builder.addOffset(0, stopsOffset, 0); }
  public static int createStopsVector(FlatBufferBuilder builder, int[] data) { builder.startVector(4, data.length, 4); for (int i = data.length - 1; i >= 0; i--) builder.addOffset(data[i]); return builder.endVector(); }
  public static void startStopsVector(FlatBufferBuilder builder, int numElems) { builder.startVector(4, numElems, 4); }
  public static void addTransports(FlatBufferBuilder builder, int transportsOffset) { builder.addOffset(1, transportsOffset, 0); }
  public static int createTransportsVector(FlatBufferBuilder builder, int[] data) { builder.startVector(4, data.length, 4); for (int i = data.length - 1; i >= 0; i--) builder.addOffset(data[i]); return builder.endVector(); }
  public static void startTransportsVector(FlatBufferBuilder builder, int numElems) { builder.startVector(4, numElems, 4); }
  public static void addAttributes(FlatBufferBuilder builder, int attributesOffset) { builder.addOffset(2, attributesOffset, 0); }
  public static int createAttributesVector(FlatBufferBuilder builder, int[] data) { builder.startVector(4, data.length, 4); for (int i = data.length - 1; i >= 0; i--) builder.addOffset(data[i]); return builder.endVector(); }
  public static void startAttributesVector(FlatBufferBuilder builder, int numElems) { builder.startVector(4, numElems, 4); }
  public static void addNightPenalty(FlatBufferBuilder builder, int nightPenalty) { builder.addShort(3, (short)(nightPenalty & 0xFFFF), 0); }
  public static int endConnection(FlatBufferBuilder builder) {
    int o = builder.endObject();
    return o;
  }
};

