// automatically generated, do not modify

package motis.bikesharing;

import java.nio.*;
import java.lang.*;
import java.util.*;
import com.google.flatbuffers.*;

public final class BikesharingRequest extends Table {
  public static BikesharingRequest getRootAsBikesharingRequest(ByteBuffer _bb) { return getRootAsBikesharingRequest(_bb, new BikesharingRequest()); }
  public static BikesharingRequest getRootAsBikesharingRequest(ByteBuffer _bb, BikesharingRequest obj) { _bb.order(ByteOrder.LITTLE_ENDIAN); return (obj.__init(_bb.getInt(_bb.position()) + _bb.position(), _bb)); }
  public BikesharingRequest __init(int _i, ByteBuffer _bb) { bb_pos = _i; bb = _bb; return this; }

  public double departureLat() { int o = __offset(4); return o != 0 ? bb.getDouble(o + bb_pos) : 0; }
  public double departureLng() { int o = __offset(6); return o != 0 ? bb.getDouble(o + bb_pos) : 0; }
  public double arrivalLat() { int o = __offset(8); return o != 0 ? bb.getDouble(o + bb_pos) : 0; }
  public double arrivalLng() { int o = __offset(10); return o != 0 ? bb.getDouble(o + bb_pos) : 0; }
  public long windowBegin() { int o = __offset(12); return o != 0 ? bb.getLong(o + bb_pos) : 0; }
  public long windowEnd() { int o = __offset(14); return o != 0 ? bb.getLong(o + bb_pos) : 0; }
  public int availabilityAggregator() { int o = __offset(16); return o != 0 ? bb.get(o + bb_pos) & 0xFF : 0; }

  public static int createBikesharingRequest(FlatBufferBuilder builder,
      double departure_lat,
      double departure_lng,
      double arrival_lat,
      double arrival_lng,
      long window_begin,
      long window_end,
      int availability_aggregator) {
    builder.startObject(7);
    BikesharingRequest.addWindowEnd(builder, window_end);
    BikesharingRequest.addWindowBegin(builder, window_begin);
    BikesharingRequest.addArrivalLng(builder, arrival_lng);
    BikesharingRequest.addArrivalLat(builder, arrival_lat);
    BikesharingRequest.addDepartureLng(builder, departure_lng);
    BikesharingRequest.addDepartureLat(builder, departure_lat);
    BikesharingRequest.addAvailabilityAggregator(builder, availability_aggregator);
    return BikesharingRequest.endBikesharingRequest(builder);
  }

  public static void startBikesharingRequest(FlatBufferBuilder builder) { builder.startObject(7); }
  public static void addDepartureLat(FlatBufferBuilder builder, double departureLat) { builder.addDouble(0, departureLat, 0); }
  public static void addDepartureLng(FlatBufferBuilder builder, double departureLng) { builder.addDouble(1, departureLng, 0); }
  public static void addArrivalLat(FlatBufferBuilder builder, double arrivalLat) { builder.addDouble(2, arrivalLat, 0); }
  public static void addArrivalLng(FlatBufferBuilder builder, double arrivalLng) { builder.addDouble(3, arrivalLng, 0); }
  public static void addWindowBegin(FlatBufferBuilder builder, long windowBegin) { builder.addLong(4, windowBegin, 0); }
  public static void addWindowEnd(FlatBufferBuilder builder, long windowEnd) { builder.addLong(5, windowEnd, 0); }
  public static void addAvailabilityAggregator(FlatBufferBuilder builder, int availabilityAggregator) { builder.addByte(6, (byte)(availabilityAggregator & 0xFF), 0); }
  public static int endBikesharingRequest(FlatBufferBuilder builder) {
    int o = builder.endObject();
    return o;
  }
};

