// automatically generated, do not modify

package motis.lookup;

import java.nio.*;
import java.lang.*;
import java.util.*;
import com.google.flatbuffers.*;

public final class LookupBatchGeoStationRequest extends Table {
  public static LookupBatchGeoStationRequest getRootAsLookupBatchGeoStationRequest(ByteBuffer _bb) { return getRootAsLookupBatchGeoStationRequest(_bb, new LookupBatchGeoStationRequest()); }
  public static LookupBatchGeoStationRequest getRootAsLookupBatchGeoStationRequest(ByteBuffer _bb, LookupBatchGeoStationRequest obj) { _bb.order(ByteOrder.LITTLE_ENDIAN); return (obj.__init(_bb.getInt(_bb.position()) + _bb.position(), _bb)); }
  public LookupBatchGeoStationRequest __init(int _i, ByteBuffer _bb) { bb_pos = _i; bb = _bb; return this; }

  public LookupGeoStationRequest requests(int j) { return requests(new LookupGeoStationRequest(), j); }
  public LookupGeoStationRequest requests(LookupGeoStationRequest obj, int j) { int o = __offset(4); return o != 0 ? obj.__init(__indirect(__vector(o) + j * 4), bb) : null; }
  public int requestsLength() { int o = __offset(4); return o != 0 ? __vector_len(o) : 0; }

  public static int createLookupBatchGeoStationRequest(FlatBufferBuilder builder,
      int requests) {
    builder.startObject(1);
    LookupBatchGeoStationRequest.addRequests(builder, requests);
    return LookupBatchGeoStationRequest.endLookupBatchGeoStationRequest(builder);
  }

  public static void startLookupBatchGeoStationRequest(FlatBufferBuilder builder) { builder.startObject(1); }
  public static void addRequests(FlatBufferBuilder builder, int requestsOffset) { builder.addOffset(0, requestsOffset, 0); }
  public static int createRequestsVector(FlatBufferBuilder builder, int[] data) { builder.startVector(4, data.length, 4); for (int i = data.length - 1; i >= 0; i--) builder.addOffset(data[i]); return builder.endVector(); }
  public static void startRequestsVector(FlatBufferBuilder builder, int numElems) { builder.startVector(4, numElems, 4); }
  public static int endLookupBatchGeoStationRequest(FlatBufferBuilder builder) {
    int o = builder.endObject();
    return o;
  }
};

