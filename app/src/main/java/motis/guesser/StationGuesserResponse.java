// automatically generated, do not modify

package motis.guesser;

import java.nio.*;
import java.lang.*;
import java.util.*;
import com.google.flatbuffers.*;

public final class StationGuesserResponse extends Table {
  public static StationGuesserResponse getRootAsStationGuesserResponse(ByteBuffer _bb) { return getRootAsStationGuesserResponse(_bb, new StationGuesserResponse()); }
  public static StationGuesserResponse getRootAsStationGuesserResponse(ByteBuffer _bb, StationGuesserResponse obj) { _bb.order(ByteOrder.LITTLE_ENDIAN); return (obj.__init(_bb.getInt(_bb.position()) + _bb.position(), _bb)); }
  public StationGuesserResponse __init(int _i, ByteBuffer _bb) { bb_pos = _i; bb = _bb; return this; }

  public Station guesses(int j) { return guesses(new Station(), j); }
  public Station guesses(Station obj, int j) { int o = __offset(4); return o != 0 ? obj.__init(__indirect(__vector(o) + j * 4), bb) : null; }
  public int guessesLength() { int o = __offset(4); return o != 0 ? __vector_len(o) : 0; }

  public static int createStationGuesserResponse(FlatBufferBuilder builder,
      int guesses) {
    builder.startObject(1);
    StationGuesserResponse.addGuesses(builder, guesses);
    return StationGuesserResponse.endStationGuesserResponse(builder);
  }

  public static void startStationGuesserResponse(FlatBufferBuilder builder) { builder.startObject(1); }
  public static void addGuesses(FlatBufferBuilder builder, int guessesOffset) { builder.addOffset(0, guessesOffset, 0); }
  public static int createGuessesVector(FlatBufferBuilder builder, int[] data) { builder.startVector(4, data.length, 4); for (int i = data.length - 1; i >= 0; i--) builder.addOffset(data[i]); return builder.endVector(); }
  public static void startGuessesVector(FlatBufferBuilder builder, int numElems) { builder.startVector(4, numElems, 4); }
  public static int endStationGuesserResponse(FlatBufferBuilder builder) {
    int o = builder.endObject();
    return o;
  }
};

