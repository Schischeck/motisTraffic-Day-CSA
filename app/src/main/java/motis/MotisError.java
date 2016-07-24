// automatically generated, do not modify

package motis;

import java.nio.*;
import java.lang.*;
import java.util.*;
import com.google.flatbuffers.*;

public final class MotisError extends Table {
  public static MotisError getRootAsMotisError(ByteBuffer _bb) { return getRootAsMotisError(_bb, new MotisError()); }
  public static MotisError getRootAsMotisError(ByteBuffer _bb, MotisError obj) { _bb.order(ByteOrder.LITTLE_ENDIAN); return (obj.__init(_bb.getInt(_bb.position()) + _bb.position(), _bb)); }
  public MotisError __init(int _i, ByteBuffer _bb) { bb_pos = _i; bb = _bb; return this; }

  public int errorCode() { int o = __offset(4); return o != 0 ? bb.getShort(o + bb_pos) & 0xFFFF : 0; }
  public String category() { int o = __offset(6); return o != 0 ? __string(o + bb_pos) : null; }
  public ByteBuffer categoryAsByteBuffer() { return __vector_as_bytebuffer(6, 1); }
  public String reason() { int o = __offset(8); return o != 0 ? __string(o + bb_pos) : null; }
  public ByteBuffer reasonAsByteBuffer() { return __vector_as_bytebuffer(8, 1); }

  public static int createMotisError(FlatBufferBuilder builder,
      int error_code,
      int category,
      int reason) {
    builder.startObject(3);
    MotisError.addReason(builder, reason);
    MotisError.addCategory(builder, category);
    MotisError.addErrorCode(builder, error_code);
    return MotisError.endMotisError(builder);
  }

  public static void startMotisError(FlatBufferBuilder builder) { builder.startObject(3); }
  public static void addErrorCode(FlatBufferBuilder builder, int errorCode) { builder.addShort(0, (short)(errorCode & 0xFFFF), 0); }
  public static void addCategory(FlatBufferBuilder builder, int categoryOffset) { builder.addOffset(1, categoryOffset, 0); }
  public static void addReason(FlatBufferBuilder builder, int reasonOffset) { builder.addOffset(2, reasonOffset, 0); }
  public static int endMotisError(FlatBufferBuilder builder) {
    int o = builder.endObject();
    return o;
  }
};

