// automatically generated, do not modify

package motis.bikesharing;

import java.nio.*;
import java.lang.*;
import java.util.*;
import com.google.flatbuffers.*;

public final class BikesharingEdge extends Table {
  public static BikesharingEdge getRootAsBikesharingEdge(ByteBuffer _bb) { return getRootAsBikesharingEdge(_bb, new BikesharingEdge()); }
  public static BikesharingEdge getRootAsBikesharingEdge(ByteBuffer _bb, BikesharingEdge obj) { _bb.order(ByteOrder.LITTLE_ENDIAN); return (obj.__init(_bb.getInt(_bb.position()) + _bb.position(), _bb)); }
  public BikesharingEdge __init(int _i, ByteBuffer _bb) { bb_pos = _i; bb = _bb; return this; }

  public BikesharingTerminal from() { return from(new BikesharingTerminal()); }
  public BikesharingTerminal from(BikesharingTerminal obj) { int o = __offset(4); return o != 0 ? obj.__init(__indirect(o + bb_pos), bb) : null; }
  public BikesharingTerminal to() { return to(new BikesharingTerminal()); }
  public BikesharingTerminal to(BikesharingTerminal obj) { int o = __offset(6); return o != 0 ? obj.__init(__indirect(o + bb_pos), bb) : null; }
  public BikesharingAvailability availability(int j) { return availability(new BikesharingAvailability(), j); }
  public BikesharingAvailability availability(BikesharingAvailability obj, int j) { int o = __offset(8); return o != 0 ? obj.__init(__vector(o) + j * 24, bb) : null; }
  public int availabilityLength() { int o = __offset(8); return o != 0 ? __vector_len(o) : 0; }
  public String evaNr() { int o = __offset(10); return o != 0 ? __string(o + bb_pos) : null; }
  public ByteBuffer evaNrAsByteBuffer() { return __vector_as_bytebuffer(10, 1); }
  public long walkDuration() { int o = __offset(12); return o != 0 ? bb.getLong(o + bb_pos) : 0; }
  public long bikeDuration() { int o = __offset(14); return o != 0 ? bb.getLong(o + bb_pos) : 0; }

  public static int createBikesharingEdge(FlatBufferBuilder builder,
      int from,
      int to,
      int availability,
      int eva_nr,
      long walk_duration,
      long bike_duration) {
    builder.startObject(6);
    BikesharingEdge.addBikeDuration(builder, bike_duration);
    BikesharingEdge.addWalkDuration(builder, walk_duration);
    BikesharingEdge.addEvaNr(builder, eva_nr);
    BikesharingEdge.addAvailability(builder, availability);
    BikesharingEdge.addTo(builder, to);
    BikesharingEdge.addFrom(builder, from);
    return BikesharingEdge.endBikesharingEdge(builder);
  }

  public static void startBikesharingEdge(FlatBufferBuilder builder) { builder.startObject(6); }
  public static void addFrom(FlatBufferBuilder builder, int fromOffset) { builder.addOffset(0, fromOffset, 0); }
  public static void addTo(FlatBufferBuilder builder, int toOffset) { builder.addOffset(1, toOffset, 0); }
  public static void addAvailability(FlatBufferBuilder builder, int availabilityOffset) { builder.addOffset(2, availabilityOffset, 0); }
  public static void startAvailabilityVector(FlatBufferBuilder builder, int numElems) { builder.startVector(24, numElems, 8); }
  public static void addEvaNr(FlatBufferBuilder builder, int evaNrOffset) { builder.addOffset(3, evaNrOffset, 0); }
  public static void addWalkDuration(FlatBufferBuilder builder, long walkDuration) { builder.addLong(4, walkDuration, 0); }
  public static void addBikeDuration(FlatBufferBuilder builder, long bikeDuration) { builder.addLong(5, bikeDuration, 0); }
  public static int endBikesharingEdge(FlatBufferBuilder builder) {
    int o = builder.endObject();
    return o;
  }
};

