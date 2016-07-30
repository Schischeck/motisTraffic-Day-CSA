// automatically generated, do not modify

package motis.lookup;

import java.nio.*;
import java.lang.*;
import java.util.*;
import com.google.flatbuffers.*;

public final class LookupGeoStationRequest extends Table {
  public static LookupGeoStationRequest getRootAsLookupGeoStationRequest(ByteBuffer _bb) { return getRootAsLookupGeoStationRequest(_bb, new LookupGeoStationRequest()); }
  public static LookupGeoStationRequest getRootAsLookupGeoStationRequest(ByteBuffer _bb, LookupGeoStationRequest obj) { _bb.order(ByteOrder.LITTLE_ENDIAN); return (obj.__init(_bb.getInt(_bb.position()) + _bb.position(), _bb)); }
  public LookupGeoStationRequest __init(int _i, ByteBuffer _bb) { bb_pos = _i; bb = _bb; return this; }

  public double lat() { int o = __offset(4); return o != 0 ? bb.getDouble(o + bb_pos) : 0; }
  public double lng() { int o = __offset(6); return o != 0 ? bb.getDouble(o + bb_pos) : 0; }
  public double radius() { int o = __offset(8); return o != 0 ? bb.getDouble(o + bb_pos) : 0; }

  public static int createLookupGeoStationRequest(FlatBufferBuilder builder,
      double lat,
      double lng,
      double radius) {
    builder.startObject(3);
    LookupGeoStationRequest.addRadius(builder, radius);
    LookupGeoStationRequest.addLng(builder, lng);
    LookupGeoStationRequest.addLat(builder, lat);
    return LookupGeoStationRequest.endLookupGeoStationRequest(builder);
  }

  public static void startLookupGeoStationRequest(FlatBufferBuilder builder) { builder.startObject(3); }
  public static void addLat(FlatBufferBuilder builder, double lat) { builder.addDouble(0, lat, 0); }
  public static void addLng(FlatBufferBuilder builder, double lng) { builder.addDouble(1, lng, 0); }
  public static void addRadius(FlatBufferBuilder builder, double radius) { builder.addDouble(2, radius, 0); }
  public static int endLookupGeoStationRequest(FlatBufferBuilder builder) {
    int o = builder.endObject();
    return o;
  }
};

