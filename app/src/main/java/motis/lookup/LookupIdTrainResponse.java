// automatically generated, do not modify

package motis.lookup;

import java.nio.*;
import java.lang.*;
import java.util.*;
import com.google.flatbuffers.*;

import motis.routing.Connection;

public final class LookupIdTrainResponse extends Table {
  public static LookupIdTrainResponse getRootAsLookupIdTrainResponse(ByteBuffer _bb) { return getRootAsLookupIdTrainResponse(_bb, new LookupIdTrainResponse()); }
  public static LookupIdTrainResponse getRootAsLookupIdTrainResponse(ByteBuffer _bb, LookupIdTrainResponse obj) { _bb.order(ByteOrder.LITTLE_ENDIAN); return (obj.__init(_bb.getInt(_bb.position()) + _bb.position(), _bb)); }
  public LookupIdTrainResponse __init(int _i, ByteBuffer _bb) { bb_pos = _i; bb = _bb; return this; }

  public Connection train() { return train(new Connection()); }
  public Connection train(Connection obj) { int o = __offset(4); return o != 0 ? obj.__init(__indirect(o + bb_pos), bb) : null; }

  public static int createLookupIdTrainResponse(FlatBufferBuilder builder,
      int train) {
    builder.startObject(1);
    LookupIdTrainResponse.addTrain(builder, train);
    return LookupIdTrainResponse.endLookupIdTrainResponse(builder);
  }

  public static void startLookupIdTrainResponse(FlatBufferBuilder builder) { builder.startObject(1); }
  public static void addTrain(FlatBufferBuilder builder, int trainOffset) { builder.addOffset(0, trainOffset, 0); }
  public static int endLookupIdTrainResponse(FlatBufferBuilder builder) {
    int o = builder.endObject();
    return o;
  }
};

