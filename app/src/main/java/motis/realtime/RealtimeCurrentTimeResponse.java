// automatically generated, do not modify

package motis.realtime;

import java.nio.*;
import java.lang.*;
import java.util.*;
import com.google.flatbuffers.*;

/**
 * Sent in response to a RealtimeCurrentTimeRequest.
 */
public final class RealtimeCurrentTimeResponse extends Table {
  public static RealtimeCurrentTimeResponse getRootAsRealtimeCurrentTimeResponse(ByteBuffer _bb) { return getRootAsRealtimeCurrentTimeResponse(_bb, new RealtimeCurrentTimeResponse()); }
  public static RealtimeCurrentTimeResponse getRootAsRealtimeCurrentTimeResponse(ByteBuffer _bb, RealtimeCurrentTimeResponse obj) { _bb.order(ByteOrder.LITTLE_ENDIAN); return (obj.__init(_bb.getInt(_bb.position()) + _bb.position(), _bb)); }
  public RealtimeCurrentTimeResponse __init(int _i, ByteBuffer _bb) { bb_pos = _i; bb = _bb; return this; }

  /**
   * Unix timestamp of the last realtime message (release time) that was
   * processed. 0 if no messages have been processed so far.
   */
  public long lastMessageTime() { int o = __offset(4); return o != 0 ? bb.getLong(o + bb_pos) : 0; }
  /**
   * Unix timestamp of the message stream start time (i.e. no messages with
   * release time below this value were processed)
   */
  public long streamStartTime() { int o = __offset(6); return o != 0 ? bb.getLong(o + bb_pos) : 0; }
  /**
   * Unix timestamp of the message stream end time (i.e. no messages with
   * release time above this value were processed)
   */
  public long streamEndTime() { int o = __offset(8); return o != 0 ? bb.getLong(o + bb_pos) : 0; }

  public static int createRealtimeCurrentTimeResponse(FlatBufferBuilder builder,
      long last_message_time,
      long stream_start_time,
      long stream_end_time) {
    builder.startObject(3);
    RealtimeCurrentTimeResponse.addStreamEndTime(builder, stream_end_time);
    RealtimeCurrentTimeResponse.addStreamStartTime(builder, stream_start_time);
    RealtimeCurrentTimeResponse.addLastMessageTime(builder, last_message_time);
    return RealtimeCurrentTimeResponse.endRealtimeCurrentTimeResponse(builder);
  }

  public static void startRealtimeCurrentTimeResponse(FlatBufferBuilder builder) { builder.startObject(3); }
  public static void addLastMessageTime(FlatBufferBuilder builder, long lastMessageTime) { builder.addLong(0, lastMessageTime, 0); }
  public static void addStreamStartTime(FlatBufferBuilder builder, long streamStartTime) { builder.addLong(1, streamStartTime, 0); }
  public static void addStreamEndTime(FlatBufferBuilder builder, long streamEndTime) { builder.addLong(2, streamEndTime, 0); }
  public static int endRealtimeCurrentTimeResponse(FlatBufferBuilder builder) {
    int o = builder.endObject();
    return o;
  }
};

