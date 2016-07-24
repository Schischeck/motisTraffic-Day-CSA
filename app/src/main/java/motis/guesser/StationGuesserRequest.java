// automatically generated, do not modify

package motis.guesser;

import java.nio.*;
import java.lang.*;
import java.util.*;
import com.google.flatbuffers.*;

public final class StationGuesserRequest extends Table {
  public static StationGuesserRequest getRootAsStationGuesserRequest(ByteBuffer _bb) { return getRootAsStationGuesserRequest(_bb, new StationGuesserRequest()); }
  public static StationGuesserRequest getRootAsStationGuesserRequest(ByteBuffer _bb, StationGuesserRequest obj) { _bb.order(ByteOrder.LITTLE_ENDIAN); return (obj.__init(_bb.getInt(_bb.position()) + _bb.position(), _bb)); }
  public StationGuesserRequest __init(int _i, ByteBuffer _bb) { bb_pos = _i; bb = _bb; return this; }

  public int guessCount() { int o = __offset(4); return o != 0 ? bb.getShort(o + bb_pos) & 0xFFFF : 8; }
  public String input() { int o = __offset(6); return o != 0 ? __string(o + bb_pos) : null; }
  public ByteBuffer inputAsByteBuffer() { return __vector_as_bytebuffer(6, 1); }

  public static int createStationGuesserRequest(FlatBufferBuilder builder,
      int guess_count,
      int input) {
    builder.startObject(2);
    StationGuesserRequest.addInput(builder, input);
    StationGuesserRequest.addGuessCount(builder, guess_count);
    return StationGuesserRequest.endStationGuesserRequest(builder);
  }

  public static void startStationGuesserRequest(FlatBufferBuilder builder) { builder.startObject(2); }
  public static void addGuessCount(FlatBufferBuilder builder, int guessCount) { builder.addShort(0, (short)(guessCount & 0xFFFF), 8); }
  public static void addInput(FlatBufferBuilder builder, int inputOffset) { builder.addOffset(1, inputOffset, 0); }
  public static int endStationGuesserRequest(FlatBufferBuilder builder) {
    int o = builder.endObject();
    return o;
  }
};

