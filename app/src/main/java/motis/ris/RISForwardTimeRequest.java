// automatically generated, do not modify

package motis.ris;

import java.nio.*;
import java.lang.*;
import java.util.*;
import com.google.flatbuffers.*;

/**
 * Forward the realtime message stream to the given time, i.e. load and process
 * all messages with a release time up to (and including) the given timestamp.
 */
public final class RISForwardTimeRequest extends Table {
  public static RISForwardTimeRequest getRootAsRISForwardTimeRequest(ByteBuffer _bb) { return getRootAsRISForwardTimeRequest(_bb, new RISForwardTimeRequest()); }
  public static RISForwardTimeRequest getRootAsRISForwardTimeRequest(ByteBuffer _bb, RISForwardTimeRequest obj) { _bb.order(ByteOrder.LITTLE_ENDIAN); return (obj.__init(_bb.getInt(_bb.position()) + _bb.position(), _bb)); }
  public RISForwardTimeRequest __init(int _i, ByteBuffer _bb) { bb_pos = _i; bb = _bb; return this; }

  /**
   * Unix timestamp - all messages with a release time up to and including
   * this timestamp are loaded and processed.
   */
  public long newTime() { int o = __offset(4); return o != 0 ? bb.getLong(o + bb_pos) : 0; }

  public static int createRISForwardTimeRequest(FlatBufferBuilder builder,
      long new_time) {
    builder.startObject(1);
    RISForwardTimeRequest.addNewTime(builder, new_time);
    return RISForwardTimeRequest.endRISForwardTimeRequest(builder);
  }

  public static void startRISForwardTimeRequest(FlatBufferBuilder builder) { builder.startObject(1); }
  public static void addNewTime(FlatBufferBuilder builder, long newTime) { builder.addLong(0, newTime, 0); }
  public static int endRISForwardTimeRequest(FlatBufferBuilder builder) {
    int o = builder.endObject();
    return o;
  }
};

