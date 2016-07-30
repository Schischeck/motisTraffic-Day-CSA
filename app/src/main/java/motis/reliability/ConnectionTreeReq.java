// automatically generated, do not modify

package motis.reliability;

import java.nio.*;
import java.lang.*;
import java.util.*;
import com.google.flatbuffers.*;

public final class ConnectionTreeReq extends Table {
  public static ConnectionTreeReq getRootAsConnectionTreeReq(ByteBuffer _bb) { return getRootAsConnectionTreeReq(_bb, new ConnectionTreeReq()); }
  public static ConnectionTreeReq getRootAsConnectionTreeReq(ByteBuffer _bb, ConnectionTreeReq obj) { _bb.order(ByteOrder.LITTLE_ENDIAN); return (obj.__init(_bb.getInt(_bb.position()) + _bb.position(), _bb)); }
  public ConnectionTreeReq __init(int _i, ByteBuffer _bb) { bb_pos = _i; bb = _bb; return this; }

  public short numAlternativesAtEachStop() { int o = __offset(4); return o != 0 ? bb.getShort(o + bb_pos) : 0; }
  public short minDepartureDiff() { int o = __offset(6); return o != 0 ? bb.getShort(o + bb_pos) : 0; }

  public static int createConnectionTreeReq(FlatBufferBuilder builder,
      short num_alternatives_at_each_stop,
      short min_departure_diff) {
    builder.startObject(2);
    ConnectionTreeReq.addMinDepartureDiff(builder, min_departure_diff);
    ConnectionTreeReq.addNumAlternativesAtEachStop(builder, num_alternatives_at_each_stop);
    return ConnectionTreeReq.endConnectionTreeReq(builder);
  }

  public static void startConnectionTreeReq(FlatBufferBuilder builder) { builder.startObject(2); }
  public static void addNumAlternativesAtEachStop(FlatBufferBuilder builder, short numAlternativesAtEachStop) { builder.addShort(0, numAlternativesAtEachStop, 0); }
  public static void addMinDepartureDiff(FlatBufferBuilder builder, short minDepartureDiff) { builder.addShort(1, minDepartureDiff, 0); }
  public static int endConnectionTreeReq(FlatBufferBuilder builder) {
    int o = builder.endObject();
    return o;
  }
};

