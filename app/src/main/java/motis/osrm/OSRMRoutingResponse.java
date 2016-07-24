// automatically generated, do not modify

package motis.osrm;

import java.nio.*;
import java.lang.*;
import java.util.*;
import com.google.flatbuffers.*;

public final class OSRMRoutingResponse extends Table {
  public static OSRMRoutingResponse getRootAsOSRMRoutingResponse(ByteBuffer _bb) { return getRootAsOSRMRoutingResponse(_bb, new OSRMRoutingResponse()); }
  public static OSRMRoutingResponse getRootAsOSRMRoutingResponse(ByteBuffer _bb, OSRMRoutingResponse obj) { _bb.order(ByteOrder.LITTLE_ENDIAN); return (obj.__init(_bb.getInt(_bb.position()) + _bb.position(), _bb)); }
  public OSRMRoutingResponse __init(int _i, ByteBuffer _bb) { bb_pos = _i; bb = _bb; return this; }

  public Cost costs(int j) { return costs(new Cost(), j); }
  public Cost costs(Cost obj, int j) { int o = __offset(4); return o != 0 ? obj.__init(__vector(o) + j * 16, bb) : null; }
  public int costsLength() { int o = __offset(4); return o != 0 ? __vector_len(o) : 0; }

  public static int createOSRMRoutingResponse(FlatBufferBuilder builder,
      int costs) {
    builder.startObject(1);
    OSRMRoutingResponse.addCosts(builder, costs);
    return OSRMRoutingResponse.endOSRMRoutingResponse(builder);
  }

  public static void startOSRMRoutingResponse(FlatBufferBuilder builder) { builder.startObject(1); }
  public static void addCosts(FlatBufferBuilder builder, int costsOffset) { builder.addOffset(0, costsOffset, 0); }
  public static void startCostsVector(FlatBufferBuilder builder, int numElems) { builder.startVector(16, numElems, 8); }
  public static int endOSRMRoutingResponse(FlatBufferBuilder builder) {
    int o = builder.endObject();
    return o;
  }
};

