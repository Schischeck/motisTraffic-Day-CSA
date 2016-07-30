// automatically generated, do not modify

package motis.lookup;

import java.nio.*;
import java.lang.*;
import java.util.*;
import com.google.flatbuffers.*;

public final class LookupGeoStationResponse extends Table {
  public static LookupGeoStationResponse getRootAsLookupGeoStationResponse(ByteBuffer _bb) { return getRootAsLookupGeoStationResponse(_bb, new LookupGeoStationResponse()); }
  public static LookupGeoStationResponse getRootAsLookupGeoStationResponse(ByteBuffer _bb, LookupGeoStationResponse obj) { _bb.order(ByteOrder.LITTLE_ENDIAN); return (obj.__init(_bb.getInt(_bb.position()) + _bb.position(), _bb)); }
  public LookupGeoStationResponse __init(int _i, ByteBuffer _bb) { bb_pos = _i; bb = _bb; return this; }

  public Station stations(int j) { return stations(new Station(), j); }
  public Station stations(Station obj, int j) { int o = __offset(4); return o != 0 ? obj.__init(__indirect(__vector(o) + j * 4), bb) : null; }
  public int stationsLength() { int o = __offset(4); return o != 0 ? __vector_len(o) : 0; }

  public static int createLookupGeoStationResponse(FlatBufferBuilder builder,
      int stations) {
    builder.startObject(1);
    LookupGeoStationResponse.addStations(builder, stations);
    return LookupGeoStationResponse.endLookupGeoStationResponse(builder);
  }

  public static void startLookupGeoStationResponse(FlatBufferBuilder builder) { builder.startObject(1); }
  public static void addStations(FlatBufferBuilder builder, int stationsOffset) { builder.addOffset(0, stationsOffset, 0); }
  public static int createStationsVector(FlatBufferBuilder builder, int[] data) { builder.startVector(4, data.length, 4); for (int i = data.length - 1; i >= 0; i--) builder.addOffset(data[i]); return builder.endVector(); }
  public static void startStationsVector(FlatBufferBuilder builder, int numElems) { builder.startVector(4, numElems, 4); }
  public static int endLookupGeoStationResponse(FlatBufferBuilder builder) {
    int o = builder.endObject();
    return o;
  }
};

