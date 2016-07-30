// automatically generated, do not modify

package motis.osrm;

import java.nio.*;
import java.lang.*;
import java.util.*;
import com.google.flatbuffers.*;

public final class OSRMRoutingRequest extends Table {
  public static OSRMRoutingRequest getRootAsOSRMRoutingRequest(ByteBuffer _bb) { return getRootAsOSRMRoutingRequest(_bb, new OSRMRoutingRequest()); }
  public static OSRMRoutingRequest getRootAsOSRMRoutingRequest(ByteBuffer _bb, OSRMRoutingRequest obj) { _bb.order(ByteOrder.LITTLE_ENDIAN); return (obj.__init(_bb.getInt(_bb.position()) + _bb.position(), _bb)); }
  public OSRMRoutingRequest __init(int _i, ByteBuffer _bb) { bb_pos = _i; bb = _bb; return this; }

  public byte direction() { int o = __offset(4); return o != 0 ? bb.get(o + bb_pos) : 0; }
  public OSRMLocation one() { return one(new OSRMLocation()); }
  public OSRMLocation one(OSRMLocation obj) { int o = __offset(6); return o != 0 ? obj.__init(o + bb_pos, bb) : null; }
  public OSRMLocation many(int j) { return many(new OSRMLocation(), j); }
  public OSRMLocation many(OSRMLocation obj, int j) { int o = __offset(8); return o != 0 ? obj.__init(__vector(o) + j * 16, bb) : null; }
  public int manyLength() { int o = __offset(8); return o != 0 ? __vector_len(o) : 0; }

  public static void startOSRMRoutingRequest(FlatBufferBuilder builder) { builder.startObject(3); }
  public static void addDirection(FlatBufferBuilder builder, byte direction) { builder.addByte(0, direction, 0); }
  public static void addOne(FlatBufferBuilder builder, int oneOffset) { builder.addStruct(1, oneOffset, 0); }
  public static void addMany(FlatBufferBuilder builder, int manyOffset) { builder.addOffset(2, manyOffset, 0); }
  public static void startManyVector(FlatBufferBuilder builder, int numElems) { builder.startVector(16, numElems, 8); }
  public static int endOSRMRoutingRequest(FlatBufferBuilder builder) {
    int o = builder.endObject();
    return o;
  }
};

