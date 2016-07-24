// automatically generated, do not modify

package motis.railviz;

import java.nio.*;
import java.lang.*;
import java.util.*;
import com.google.flatbuffers.*;

public final class RailVizFindTrain extends Table {
  public static RailVizFindTrain getRootAsRailVizFindTrain(ByteBuffer _bb) { return getRootAsRailVizFindTrain(_bb, new RailVizFindTrain()); }
  public static RailVizFindTrain getRootAsRailVizFindTrain(ByteBuffer _bb, RailVizFindTrain obj) { _bb.order(ByteOrder.LITTLE_ENDIAN); return (obj.__init(_bb.getInt(_bb.position()) + _bb.position(), _bb)); }
  public RailVizFindTrain __init(int _i, ByteBuffer _bb) { bb_pos = _i; bb = _bb; return this; }

  public String trainNumber() { int o = __offset(4); return o != 0 ? __string(o + bb_pos) : null; }
  public ByteBuffer trainNumberAsByteBuffer() { return __vector_as_bytebuffer(4, 1); }

  public static int createRailVizFindTrain(FlatBufferBuilder builder,
      int train_number) {
    builder.startObject(1);
    RailVizFindTrain.addTrainNumber(builder, train_number);
    return RailVizFindTrain.endRailVizFindTrain(builder);
  }

  public static void startRailVizFindTrain(FlatBufferBuilder builder) { builder.startObject(1); }
  public static void addTrainNumber(FlatBufferBuilder builder, int trainNumberOffset) { builder.addOffset(0, trainNumberOffset, 0); }
  public static int endRailVizFindTrain(FlatBufferBuilder builder) {
    int o = builder.endObject();
    return o;
  }
};

