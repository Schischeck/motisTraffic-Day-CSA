// automatically generated, do not modify

package motis.ris;

import java.nio.*;
import java.lang.*;
import java.util.*;
import com.google.flatbuffers.*;

public final class UpdatedEvent extends Table {
  public static UpdatedEvent getRootAsUpdatedEvent(ByteBuffer _bb) { return getRootAsUpdatedEvent(_bb, new UpdatedEvent()); }
  public static UpdatedEvent getRootAsUpdatedEvent(ByteBuffer _bb, UpdatedEvent obj) { _bb.order(ByteOrder.LITTLE_ENDIAN); return (obj.__init(_bb.getInt(_bb.position()) + _bb.position(), _bb)); }
  public UpdatedEvent __init(int _i, ByteBuffer _bb) { bb_pos = _i; bb = _bb; return this; }

  public Event base() { return base(new Event()); }
  public Event base(Event obj) { int o = __offset(4); return o != 0 ? obj.__init(__indirect(o + bb_pos), bb) : null; }
  public long updatedTime() { int o = __offset(6); return o != 0 ? bb.getLong(o + bb_pos) : 0; }

  public static int createUpdatedEvent(FlatBufferBuilder builder,
      int base,
      long updatedTime) {
    builder.startObject(2);
    UpdatedEvent.addUpdatedTime(builder, updatedTime);
    UpdatedEvent.addBase(builder, base);
    return UpdatedEvent.endUpdatedEvent(builder);
  }

  public static void startUpdatedEvent(FlatBufferBuilder builder) { builder.startObject(2); }
  public static void addBase(FlatBufferBuilder builder, int baseOffset) { builder.addOffset(0, baseOffset, 0); }
  public static void addUpdatedTime(FlatBufferBuilder builder, long updatedTime) { builder.addLong(1, updatedTime, 0); }
  public static int endUpdatedEvent(FlatBufferBuilder builder) {
    int o = builder.endObject();
    return o;
  }
};

