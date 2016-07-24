// automatically generated, do not modify

package motis;

import java.nio.*;
import java.lang.*;
import java.util.*;
import com.google.flatbuffers.*;

public final class HTTPResponse extends Table {
  public static HTTPResponse getRootAsHTTPResponse(ByteBuffer _bb) { return getRootAsHTTPResponse(_bb, new HTTPResponse()); }
  public static HTTPResponse getRootAsHTTPResponse(ByteBuffer _bb, HTTPResponse obj) { _bb.order(ByteOrder.LITTLE_ENDIAN); return (obj.__init(_bb.getInt(_bb.position()) + _bb.position(), _bb)); }
  public HTTPResponse __init(int _i, ByteBuffer _bb) { bb_pos = _i; bb = _bb; return this; }

  public byte status() { int o = __offset(4); return o != 0 ? bb.get(o + bb_pos) : 0; }
  public HTTPHeader headers(int j) { return headers(new HTTPHeader(), j); }
  public HTTPHeader headers(HTTPHeader obj, int j) { int o = __offset(6); return o != 0 ? obj.__init(__indirect(__vector(o) + j * 4), bb) : null; }
  public int headersLength() { int o = __offset(6); return o != 0 ? __vector_len(o) : 0; }
  public String content() { int o = __offset(8); return o != 0 ? __string(o + bb_pos) : null; }
  public ByteBuffer contentAsByteBuffer() { return __vector_as_bytebuffer(8, 1); }

  public static int createHTTPResponse(FlatBufferBuilder builder,
      byte status,
      int headers,
      int content) {
    builder.startObject(3);
    HTTPResponse.addContent(builder, content);
    HTTPResponse.addHeaders(builder, headers);
    HTTPResponse.addStatus(builder, status);
    return HTTPResponse.endHTTPResponse(builder);
  }

  public static void startHTTPResponse(FlatBufferBuilder builder) { builder.startObject(3); }
  public static void addStatus(FlatBufferBuilder builder, byte status) { builder.addByte(0, status, 0); }
  public static void addHeaders(FlatBufferBuilder builder, int headersOffset) { builder.addOffset(1, headersOffset, 0); }
  public static int createHeadersVector(FlatBufferBuilder builder, int[] data) { builder.startVector(4, data.length, 4); for (int i = data.length - 1; i >= 0; i--) builder.addOffset(data[i]); return builder.endVector(); }
  public static void startHeadersVector(FlatBufferBuilder builder, int numElems) { builder.startVector(4, numElems, 4); }
  public static void addContent(FlatBufferBuilder builder, int contentOffset) { builder.addOffset(2, contentOffset, 0); }
  public static int endHTTPResponse(FlatBufferBuilder builder) {
    int o = builder.endObject();
    return o;
  }
};

