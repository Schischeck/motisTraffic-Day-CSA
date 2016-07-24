// automatically generated, do not modify

package motis.routing;

import java.nio.*;
import java.lang.*;
import java.util.*;
import com.google.flatbuffers.*;

public final class Attribute extends Table {
  public static Attribute getRootAsAttribute(ByteBuffer _bb) { return getRootAsAttribute(_bb, new Attribute()); }
  public static Attribute getRootAsAttribute(ByteBuffer _bb, Attribute obj) { _bb.order(ByteOrder.LITTLE_ENDIAN); return (obj.__init(_bb.getInt(_bb.position()) + _bb.position(), _bb)); }
  public Attribute __init(int _i, ByteBuffer _bb) { bb_pos = _i; bb = _bb; return this; }

  public short from() { int o = __offset(4); return o != 0 ? bb.getShort(o + bb_pos) : 0; }
  public short to() { int o = __offset(6); return o != 0 ? bb.getShort(o + bb_pos) : 0; }
  public String code() { int o = __offset(8); return o != 0 ? __string(o + bb_pos) : null; }
  public ByteBuffer codeAsByteBuffer() { return __vector_as_bytebuffer(8, 1); }
  public String text() { int o = __offset(10); return o != 0 ? __string(o + bb_pos) : null; }
  public ByteBuffer textAsByteBuffer() { return __vector_as_bytebuffer(10, 1); }

  public static int createAttribute(FlatBufferBuilder builder,
      short from,
      short to,
      int code,
      int text) {
    builder.startObject(4);
    Attribute.addText(builder, text);
    Attribute.addCode(builder, code);
    Attribute.addTo(builder, to);
    Attribute.addFrom(builder, from);
    return Attribute.endAttribute(builder);
  }

  public static void startAttribute(FlatBufferBuilder builder) { builder.startObject(4); }
  public static void addFrom(FlatBufferBuilder builder, short from) { builder.addShort(0, from, 0); }
  public static void addTo(FlatBufferBuilder builder, short to) { builder.addShort(1, to, 0); }
  public static void addCode(FlatBufferBuilder builder, int codeOffset) { builder.addOffset(2, codeOffset, 0); }
  public static void addText(FlatBufferBuilder builder, int textOffset) { builder.addOffset(3, textOffset, 0); }
  public static int endAttribute(FlatBufferBuilder builder) {
    int o = builder.endObject();
    return o;
  }
};

