// automatically generated, do not modify

package motis.ris;

import java.nio.*;
import java.lang.*;
import java.util.*;
import com.google.flatbuffers.*;

public final class AdditionalEvent extends Table {
  public static AdditionalEvent getRootAsAdditionalEvent(ByteBuffer _bb) { return getRootAsAdditionalEvent(_bb, new AdditionalEvent()); }
  public static AdditionalEvent getRootAsAdditionalEvent(ByteBuffer _bb, AdditionalEvent obj) { _bb.order(ByteOrder.LITTLE_ENDIAN); return (obj.__init(_bb.getInt(_bb.position()) + _bb.position(), _bb)); }
  public AdditionalEvent __init(int _i, ByteBuffer _bb) { bb_pos = _i; bb = _bb; return this; }

  public Event base() { return base(new Event()); }
  public Event base(Event obj) { int o = __offset(4); return o != 0 ? obj.__init(__indirect(o + bb_pos), bb) : null; }
  public String trainCategory() { int o = __offset(6); return o != 0 ? __string(o + bb_pos) : null; }
  public ByteBuffer trainCategoryAsByteBuffer() { return __vector_as_bytebuffer(6, 1); }
  public String track() { int o = __offset(8); return o != 0 ? __string(o + bb_pos) : null; }
  public ByteBuffer trackAsByteBuffer() { return __vector_as_bytebuffer(8, 1); }

  public static int createAdditionalEvent(FlatBufferBuilder builder,
      int base,
      int trainCategory,
      int track) {
    builder.startObject(3);
    AdditionalEvent.addTrack(builder, track);
    AdditionalEvent.addTrainCategory(builder, trainCategory);
    AdditionalEvent.addBase(builder, base);
    return AdditionalEvent.endAdditionalEvent(builder);
  }

  public static void startAdditionalEvent(FlatBufferBuilder builder) { builder.startObject(3); }
  public static void addBase(FlatBufferBuilder builder, int baseOffset) { builder.addOffset(0, baseOffset, 0); }
  public static void addTrainCategory(FlatBufferBuilder builder, int trainCategoryOffset) { builder.addOffset(1, trainCategoryOffset, 0); }
  public static void addTrack(FlatBufferBuilder builder, int trackOffset) { builder.addOffset(2, trackOffset, 0); }
  public static int endAdditionalEvent(FlatBufferBuilder builder) {
    int o = builder.endObject();
    builder.required(o, 6);  // trainCategory
    return o;
  }
};

