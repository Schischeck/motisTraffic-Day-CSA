// automatically generated, do not modify

package motis.reliability;

import java.nio.*;
import java.lang.*;
import java.util.*;
import com.google.flatbuffers.*;

public final class RequestOptionsWrapper extends Table {
  public static RequestOptionsWrapper getRootAsRequestOptionsWrapper(ByteBuffer _bb) { return getRootAsRequestOptionsWrapper(_bb, new RequestOptionsWrapper()); }
  public static RequestOptionsWrapper getRootAsRequestOptionsWrapper(ByteBuffer _bb, RequestOptionsWrapper obj) { _bb.order(ByteOrder.LITTLE_ENDIAN); return (obj.__init(_bb.getInt(_bb.position()) + _bb.position(), _bb)); }
  public RequestOptionsWrapper __init(int _i, ByteBuffer _bb) { bb_pos = _i; bb = _bb; return this; }

  public byte requestOptionsType() { int o = __offset(4); return o != 0 ? bb.get(o + bb_pos) : 0; }
  public Table requestOptions(Table obj) { int o = __offset(6); return o != 0 ? __union(obj, o) : null; }

  public static int createRequestOptionsWrapper(FlatBufferBuilder builder,
      byte request_options_type,
      int request_options) {
    builder.startObject(2);
    RequestOptionsWrapper.addRequestOptions(builder, request_options);
    RequestOptionsWrapper.addRequestOptionsType(builder, request_options_type);
    return RequestOptionsWrapper.endRequestOptionsWrapper(builder);
  }

  public static void startRequestOptionsWrapper(FlatBufferBuilder builder) { builder.startObject(2); }
  public static void addRequestOptionsType(FlatBufferBuilder builder, byte requestOptionsType) { builder.addByte(0, requestOptionsType, 0); }
  public static void addRequestOptions(FlatBufferBuilder builder, int requestOptionsOffset) { builder.addOffset(1, requestOptionsOffset, 0); }
  public static int endRequestOptionsWrapper(FlatBufferBuilder builder) {
    int o = builder.endObject();
    return o;
  }
};

