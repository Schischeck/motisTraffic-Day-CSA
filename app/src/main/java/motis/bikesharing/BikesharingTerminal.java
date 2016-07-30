// automatically generated, do not modify

package motis.bikesharing;

import java.nio.*;
import java.lang.*;
import java.util.*;
import com.google.flatbuffers.*;

public final class BikesharingTerminal extends Table {
  public static BikesharingTerminal getRootAsBikesharingTerminal(ByteBuffer _bb) { return getRootAsBikesharingTerminal(_bb, new BikesharingTerminal()); }
  public static BikesharingTerminal getRootAsBikesharingTerminal(ByteBuffer _bb, BikesharingTerminal obj) { _bb.order(ByteOrder.LITTLE_ENDIAN); return (obj.__init(_bb.getInt(_bb.position()) + _bb.position(), _bb)); }
  public BikesharingTerminal __init(int _i, ByteBuffer _bb) { bb_pos = _i; bb = _bb; return this; }

  public String id() { int o = __offset(4); return o != 0 ? __string(o + bb_pos) : null; }
  public ByteBuffer idAsByteBuffer() { return __vector_as_bytebuffer(4, 1); }
  public double lat() { int o = __offset(6); return o != 0 ? bb.getDouble(o + bb_pos) : 0; }
  public double lng() { int o = __offset(8); return o != 0 ? bb.getDouble(o + bb_pos) : 0; }
  public String name() { int o = __offset(10); return o != 0 ? __string(o + bb_pos) : null; }
  public ByteBuffer nameAsByteBuffer() { return __vector_as_bytebuffer(10, 1); }

  public static int createBikesharingTerminal(FlatBufferBuilder builder,
      int id,
      double lat,
      double lng,
      int name) {
    builder.startObject(4);
    BikesharingTerminal.addLng(builder, lng);
    BikesharingTerminal.addLat(builder, lat);
    BikesharingTerminal.addName(builder, name);
    BikesharingTerminal.addId(builder, id);
    return BikesharingTerminal.endBikesharingTerminal(builder);
  }

  public static void startBikesharingTerminal(FlatBufferBuilder builder) { builder.startObject(4); }
  public static void addId(FlatBufferBuilder builder, int idOffset) { builder.addOffset(0, idOffset, 0); }
  public static void addLat(FlatBufferBuilder builder, double lat) { builder.addDouble(1, lat, 0); }
  public static void addLng(FlatBufferBuilder builder, double lng) { builder.addDouble(2, lng, 0); }
  public static void addName(FlatBufferBuilder builder, int nameOffset) { builder.addOffset(3, nameOffset, 0); }
  public static int endBikesharingTerminal(FlatBufferBuilder builder) {
    int o = builder.endObject();
    return o;
  }
};

