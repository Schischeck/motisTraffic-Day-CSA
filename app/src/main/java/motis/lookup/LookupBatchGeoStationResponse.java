// automatically generated, do not modify

package motis.lookup;

import java.nio.*;
import java.lang.*;
import java.util.*;
import com.google.flatbuffers.*;

public final class LookupBatchGeoStationResponse extends Table {
  public static LookupBatchGeoStationResponse getRootAsLookupBatchGeoStationResponse(ByteBuffer _bb) { return getRootAsLookupBatchGeoStationResponse(_bb, new LookupBatchGeoStationResponse()); }
  public static LookupBatchGeoStationResponse getRootAsLookupBatchGeoStationResponse(ByteBuffer _bb, LookupBatchGeoStationResponse obj) { _bb.order(ByteOrder.LITTLE_ENDIAN); return (obj.__init(_bb.getInt(_bb.position()) + _bb.position(), _bb)); }
  public LookupBatchGeoStationResponse __init(int _i, ByteBuffer _bb) { bb_pos = _i; bb = _bb; return this; }

  public LookupGeoStationResponse responses(int j) { return responses(new LookupGeoStationResponse(), j); }
  public LookupGeoStationResponse responses(LookupGeoStationResponse obj, int j) { int o = __offset(4); return o != 0 ? obj.__init(__indirect(__vector(o) + j * 4), bb) : null; }
  public int responsesLength() { int o = __offset(4); return o != 0 ? __vector_len(o) : 0; }

  public static int createLookupBatchGeoStationResponse(FlatBufferBuilder builder,
      int responses) {
    builder.startObject(1);
    LookupBatchGeoStationResponse.addResponses(builder, responses);
    return LookupBatchGeoStationResponse.endLookupBatchGeoStationResponse(builder);
  }

  public static void startLookupBatchGeoStationResponse(FlatBufferBuilder builder) { builder.startObject(1); }
  public static void addResponses(FlatBufferBuilder builder, int responsesOffset) { builder.addOffset(0, responsesOffset, 0); }
  public static int createResponsesVector(FlatBufferBuilder builder, int[] data) { builder.startVector(4, data.length, 4); for (int i = data.length - 1; i >= 0; i--) builder.addOffset(data[i]); return builder.endVector(); }
  public static void startResponsesVector(FlatBufferBuilder builder, int numElems) { builder.startVector(4, numElems, 4); }
  public static int endLookupBatchGeoStationResponse(FlatBufferBuilder builder) {
    int o = builder.endObject();
    return o;
  }
};

