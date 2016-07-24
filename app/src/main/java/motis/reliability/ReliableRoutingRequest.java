// automatically generated, do not modify

package motis.reliability;

import java.nio.*;
import java.lang.*;
import java.util.*;
import com.google.flatbuffers.*;

import motis.routing.RoutingRequest;

public final class ReliableRoutingRequest extends Table {
  public static ReliableRoutingRequest getRootAsReliableRoutingRequest(ByteBuffer _bb) { return getRootAsReliableRoutingRequest(_bb, new ReliableRoutingRequest()); }
  public static ReliableRoutingRequest getRootAsReliableRoutingRequest(ByteBuffer _bb, ReliableRoutingRequest obj) { _bb.order(ByteOrder.LITTLE_ENDIAN); return (obj.__init(_bb.getInt(_bb.position()) + _bb.position(), _bb)); }
  public ReliableRoutingRequest __init(int _i, ByteBuffer _bb) { bb_pos = _i; bb = _bb; return this; }

  public RoutingRequest request() { return request(new RoutingRequest()); }
  public RoutingRequest request(RoutingRequest obj) { int o = __offset(4); return o != 0 ? obj.__init(__indirect(o + bb_pos), bb) : null; }
  public RequestOptionsWrapper requestType() { return requestType(new RequestOptionsWrapper()); }
  public RequestOptionsWrapper requestType(RequestOptionsWrapper obj) { int o = __offset(6); return o != 0 ? obj.__init(__indirect(o + bb_pos), bb) : null; }

  public static int createReliableRoutingRequest(FlatBufferBuilder builder,
      int request,
      int request_type) {
    builder.startObject(2);
    ReliableRoutingRequest.addRequestType(builder, request_type);
    ReliableRoutingRequest.addRequest(builder, request);
    return ReliableRoutingRequest.endReliableRoutingRequest(builder);
  }

  public static void startReliableRoutingRequest(FlatBufferBuilder builder) { builder.startObject(2); }
  public static void addRequest(FlatBufferBuilder builder, int requestOffset) { builder.addOffset(0, requestOffset, 0); }
  public static void addRequestType(FlatBufferBuilder builder, int requestTypeOffset) { builder.addOffset(1, requestTypeOffset, 0); }
  public static int endReliableRoutingRequest(FlatBufferBuilder builder) {
    int o = builder.endObject();
    return o;
  }
};

