// automatically generated, do not modify

package motis.routing;

import java.nio.*;
import java.lang.*;
import java.util.*;
import com.google.flatbuffers.*;

public final class StationPathElement extends Table {
  public static StationPathElement getRootAsStationPathElement(ByteBuffer _bb) { return getRootAsStationPathElement(_bb, new StationPathElement()); }
  public static StationPathElement getRootAsStationPathElement(ByteBuffer _bb, StationPathElement obj) { _bb.order(ByteOrder.LITTLE_ENDIAN); return (obj.__init(_bb.getInt(_bb.position()) + _bb.position(), _bb)); }
  public StationPathElement __init(int _i, ByteBuffer _bb) { bb_pos = _i; bb = _bb; return this; }

  public String name() { int o = __offset(4); return o != 0 ? __string(o + bb_pos) : null; }
  public ByteBuffer nameAsByteBuffer() { return __vector_as_bytebuffer(4, 1); }
  public String evaNr() { int o = __offset(6); return o != 0 ? __string(o + bb_pos) : null; }
  public ByteBuffer evaNrAsByteBuffer() { return __vector_as_bytebuffer(6, 1); }

  public static int createStationPathElement(FlatBufferBuilder builder,
      int name,
      int eva_nr) {
    builder.startObject(2);
    StationPathElement.addEvaNr(builder, eva_nr);
    StationPathElement.addName(builder, name);
    return StationPathElement.endStationPathElement(builder);
  }

  public static void startStationPathElement(FlatBufferBuilder builder) { builder.startObject(2); }
  public static void addName(FlatBufferBuilder builder, int nameOffset) { builder.addOffset(0, nameOffset, 0); }
  public static void addEvaNr(FlatBufferBuilder builder, int evaNrOffset) { builder.addOffset(1, evaNrOffset, 0); }
  public static int endStationPathElement(FlatBufferBuilder builder) {
    int o = builder.endObject();
    return o;
  }
};

