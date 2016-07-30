// automatically generated, do not modify

package motis;

import java.nio.*;
import java.lang.*;
import java.util.*;
import com.google.flatbuffers.*;

public final class HTTPRequest extends Table {
  public static HTTPRequest getRootAsHTTPRequest(ByteBuffer _bb) { return getRootAsHTTPRequest(_bb, new HTTPRequest()); }
  public static HTTPRequest getRootAsHTTPRequest(ByteBuffer _bb, HTTPRequest obj) { _bb.order(ByteOrder.LITTLE_ENDIAN); return (obj.__init(_bb.getInt(_bb.position()) + _bb.position(), _bb)); }
  public HTTPRequest __init(int _i, ByteBuffer _bb) { bb_pos = _i; bb = _bb; return this; }

  public byte method() { int o = __offset(4); return o != 0 ? bb.get(o + bb_pos) : 0; }
  public String path() { int o = __offset(6); return o != 0 ? __string(o + bb_pos) : null; }
  public ByteBuffer pathAsByteBuffer() { return __vector_as_bytebuffer(6, 1); }
  public HTTPHeader headers(int j) { return headers(new HTTPHeader(), j); }
  public HTTPHeader headers(HTTPHeader obj, int j) { int o = __offset(8); return o != 0 ? obj.__init(__indirect(__vector(o) + j * 4), bb) : null; }
  public int headersLength() { int o = __offset(8); return o != 0 ? __vector_len(o) : 0; }
  public String content() { int o = __offset(10); return o != 0 ? __string(o + bb_pos) : null; }
  public ByteBuffer contentAsByteBuffer() { return __vector_as_bytebuffer(10, 1); }

  public static int createHTTPRequest(FlatBufferBuilder builder,
      byte method,
      int path,
      int headers,
      int content) {
    builder.startObject(4);
    HTTPRequest.addContent(builder, content);
    HTTPRequest.addHeaders(builder, headers);
    HTTPRequest.addPath(builder, path);
    HTTPRequest.addMethod(builder, method);
    return HTTPRequest.endHTTPRequest(builder);
  }

  public static void startHTTPRequest(FlatBufferBuilder builder) { builder.startObject(4); }
  public static void addMethod(FlatBufferBuilder builder, byte method) { builder.addByte(0, method, 0); }
  public static void addPath(FlatBufferBuilder builder, int pathOffset) { builder.addOffset(1, pathOffset, 0); }
  public static void addHeaders(FlatBufferBuilder builder, int headersOffset) { builder.addOffset(2, headersOffset, 0); }
  public static int createHeadersVector(FlatBufferBuilder builder, int[] data) { builder.startVector(4, data.length, 4); for (int i = data.length - 1; i >= 0; i--) builder.addOffset(data[i]); return builder.endVector(); }
  public static void startHeadersVector(FlatBufferBuilder builder, int numElems) { builder.startVector(4, numElems, 4); }
  public static void addContent(FlatBufferBuilder builder, int contentOffset) { builder.addOffset(3, contentOffset, 0); }
  public static int endHTTPRequest(FlatBufferBuilder builder) {
    int o = builder.endObject();
    return o;
  }
};

