// automatically generated, do not modify

package motis.routing;

import java.nio.*;
import java.lang.*;
import java.util.*;
import com.google.flatbuffers.*;

public final class Transport extends Table {
  public static Transport getRootAsTransport(ByteBuffer _bb) { return getRootAsTransport(_bb, new Transport()); }
  public static Transport getRootAsTransport(ByteBuffer _bb, Transport obj) { _bb.order(ByteOrder.LITTLE_ENDIAN); return (obj.__init(_bb.getInt(_bb.position()) + _bb.position(), _bb)); }
  public Transport __init(int _i, ByteBuffer _bb) { bb_pos = _i; bb = _bb; return this; }

  public Range range() { return range(new Range()); }
  public Range range(Range obj) { int o = __offset(4); return o != 0 ? obj.__init(o + bb_pos, bb) : null; }
  public String categoryName() { int o = __offset(6); return o != 0 ? __string(o + bb_pos) : null; }
  public ByteBuffer categoryNameAsByteBuffer() { return __vector_as_bytebuffer(6, 1); }
  public long categoryId() { int o = __offset(8); return o != 0 ? (long)bb.getInt(o + bb_pos) & 0xFFFFFFFFL : 0; }
  public long clasz() { int o = __offset(10); return o != 0 ? (long)bb.getInt(o + bb_pos) & 0xFFFFFFFFL : 0; }
  public long trainNr() { int o = __offset(12); return o != 0 ? (long)bb.getInt(o + bb_pos) & 0xFFFFFFFFL : 0; }
  public String lineId() { int o = __offset(14); return o != 0 ? __string(o + bb_pos) : null; }
  public ByteBuffer lineIdAsByteBuffer() { return __vector_as_bytebuffer(14, 1); }
  public String name() { int o = __offset(16); return o != 0 ? __string(o + bb_pos) : null; }
  public ByteBuffer nameAsByteBuffer() { return __vector_as_bytebuffer(16, 1); }
  public String provider() { int o = __offset(18); return o != 0 ? __string(o + bb_pos) : null; }
  public ByteBuffer providerAsByteBuffer() { return __vector_as_bytebuffer(18, 1); }
  public String direction() { int o = __offset(20); return o != 0 ? __string(o + bb_pos) : null; }
  public ByteBuffer directionAsByteBuffer() { return __vector_as_bytebuffer(20, 1); }
  public long routeId() { int o = __offset(22); return o != 0 ? (long)bb.getInt(o + bb_pos) & 0xFFFFFFFFL : 0; }

  public static void startTransport(FlatBufferBuilder builder) { builder.startObject(10); }
  public static void addRange(FlatBufferBuilder builder, int rangeOffset) { builder.addStruct(0, rangeOffset, 0); }
  public static void addCategoryName(FlatBufferBuilder builder, int categoryNameOffset) { builder.addOffset(1, categoryNameOffset, 0); }
  public static void addCategoryId(FlatBufferBuilder builder, long categoryId) { builder.addInt(2, (int)(categoryId & 0xFFFFFFFFL), 0); }
  public static void addClasz(FlatBufferBuilder builder, long clasz) { builder.addInt(3, (int)(clasz & 0xFFFFFFFFL), 0); }
  public static void addTrainNr(FlatBufferBuilder builder, long trainNr) { builder.addInt(4, (int)(trainNr & 0xFFFFFFFFL), 0); }
  public static void addLineId(FlatBufferBuilder builder, int lineIdOffset) { builder.addOffset(5, lineIdOffset, 0); }
  public static void addName(FlatBufferBuilder builder, int nameOffset) { builder.addOffset(6, nameOffset, 0); }
  public static void addProvider(FlatBufferBuilder builder, int providerOffset) { builder.addOffset(7, providerOffset, 0); }
  public static void addDirection(FlatBufferBuilder builder, int directionOffset) { builder.addOffset(8, directionOffset, 0); }
  public static void addRouteId(FlatBufferBuilder builder, long routeId) { builder.addInt(9, (int)(routeId & 0xFFFFFFFFL), 0); }
  public static int endTransport(FlatBufferBuilder builder) {
    int o = builder.endObject();
    return o;
  }
};

