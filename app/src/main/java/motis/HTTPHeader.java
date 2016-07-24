// automatically generated, do not modify

package motis;

import java.nio.*;
import java.lang.*;
import java.util.*;
import com.google.flatbuffers.*;

public final class HTTPHeader extends Table {
  public static HTTPHeader getRootAsHTTPHeader(ByteBuffer _bb) { return getRootAsHTTPHeader(_bb, new HTTPHeader()); }
  public static HTTPHeader getRootAsHTTPHeader(ByteBuffer _bb, HTTPHeader obj) { _bb.order(ByteOrder.LITTLE_ENDIAN); return (obj.__init(_bb.getInt(_bb.position()) + _bb.position(), _bb)); }
  public HTTPHeader __init(int _i, ByteBuffer _bb) { bb_pos = _i; bb = _bb; return this; }

  public String name() { int o = __offset(4); return o != 0 ? __string(o + bb_pos) : null; }
  public ByteBuffer nameAsByteBuffer() { return __vector_as_bytebuffer(4, 1); }
  public String value() { int o = __offset(6); return o != 0 ? __string(o + bb_pos) : null; }
  public ByteBuffer valueAsByteBuffer() { return __vector_as_bytebuffer(6, 1); }

  public static int createHTTPHeader(FlatBufferBuilder builder,
      int name,
      int value) {
    builder.startObject(2);
    HTTPHeader.addValue(builder, value);
    HTTPHeader.addName(builder, name);
    return HTTPHeader.endHTTPHeader(builder);
  }

  public static void startHTTPHeader(FlatBufferBuilder builder) { builder.startObject(2); }
  public static void addName(FlatBufferBuilder builder, int nameOffset) { builder.addOffset(0, nameOffset, 0); }
  public static void addValue(FlatBufferBuilder builder, int valueOffset) { builder.addOffset(1, valueOffset, 0); }
  public static int endHTTPHeader(FlatBufferBuilder builder) {
    int o = builder.endObject();
    return o;
  }
};

