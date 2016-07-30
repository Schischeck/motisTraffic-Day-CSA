// automatically generated, do not modify

package motis.routing;

import java.nio.*;
import java.lang.*;
import java.util.*;
import com.google.flatbuffers.*;

public final class Mumo extends Table {
  public static Mumo getRootAsMumo(ByteBuffer _bb) { return getRootAsMumo(_bb, new Mumo()); }
  public static Mumo getRootAsMumo(ByteBuffer _bb, Mumo obj) { _bb.order(ByteOrder.LITTLE_ENDIAN); return (obj.__init(_bb.getInt(_bb.position()) + _bb.position(), _bb)); }
  public Mumo __init(int _i, ByteBuffer _bb) { bb_pos = _i; bb = _bb; return this; }

  public Range range() { return range(new Range()); }
  public Range range(Range obj) { int o = __offset(4); return o != 0 ? obj.__init(o + bb_pos, bb) : null; }
  public String name() { int o = __offset(6); return o != 0 ? __string(o + bb_pos) : null; }
  public ByteBuffer nameAsByteBuffer() { return __vector_as_bytebuffer(6, 1); }
  public long price() { int o = __offset(8); return o != 0 ? (long)bb.getInt(o + bb_pos) & 0xFFFFFFFFL : 0; }

  public static void startMumo(FlatBufferBuilder builder) { builder.startObject(3); }
  public static void addRange(FlatBufferBuilder builder, int rangeOffset) { builder.addStruct(0, rangeOffset, 0); }
  public static void addName(FlatBufferBuilder builder, int nameOffset) { builder.addOffset(1, nameOffset, 0); }
  public static void addPrice(FlatBufferBuilder builder, long price) { builder.addInt(2, (int)(price & 0xFFFFFFFFL), 0); }
  public static int endMumo(FlatBufferBuilder builder) {
    int o = builder.endObject();
    return o;
  }
};

