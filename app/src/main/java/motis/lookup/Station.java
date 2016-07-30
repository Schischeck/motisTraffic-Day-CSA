// automatically generated, do not modify

package motis.lookup;

import java.nio.*;
import java.lang.*;
import java.util.*;
import com.google.flatbuffers.*;

public final class Station extends Table {
  public static Station getRootAsStation(ByteBuffer _bb) { return getRootAsStation(_bb, new Station()); }
  public static Station getRootAsStation(ByteBuffer _bb, Station obj) { _bb.order(ByteOrder.LITTLE_ENDIAN); return (obj.__init(_bb.getInt(_bb.position()) + _bb.position(), _bb)); }
  public Station __init(int _i, ByteBuffer _bb) { bb_pos = _i; bb = _bb; return this; }

  public String evaNr() { int o = __offset(4); return o != 0 ? __string(o + bb_pos) : null; }
  public ByteBuffer evaNrAsByteBuffer() { return __vector_as_bytebuffer(4, 1); }
  public String name() { int o = __offset(6); return o != 0 ? __string(o + bb_pos) : null; }
  public ByteBuffer nameAsByteBuffer() { return __vector_as_bytebuffer(6, 1); }
  public double lat() { int o = __offset(8); return o != 0 ? bb.getDouble(o + bb_pos) : 0; }
  public double lng() { int o = __offset(10); return o != 0 ? bb.getDouble(o + bb_pos) : 0; }

  public static int createStation(FlatBufferBuilder builder,
      int eva_nr,
      int name,
      double lat,
      double lng) {
    builder.startObject(4);
    Station.addLng(builder, lng);
    Station.addLat(builder, lat);
    Station.addName(builder, name);
    Station.addEvaNr(builder, eva_nr);
    return Station.endStation(builder);
  }

  public static void startStation(FlatBufferBuilder builder) { builder.startObject(4); }
  public static void addEvaNr(FlatBufferBuilder builder, int evaNrOffset) { builder.addOffset(0, evaNrOffset, 0); }
  public static void addName(FlatBufferBuilder builder, int nameOffset) { builder.addOffset(1, nameOffset, 0); }
  public static void addLat(FlatBufferBuilder builder, double lat) { builder.addDouble(2, lat, 0); }
  public static void addLng(FlatBufferBuilder builder, double lng) { builder.addDouble(3, lng, 0); }
  public static int endStation(FlatBufferBuilder builder) {
    int o = builder.endObject();
    return o;
  }
};

