// automatically generated, do not modify

package motis.railviz;

import java.nio.*;
import java.lang.*;
import java.util.*;
import com.google.flatbuffers.*;

public final class RailVizStationDetailResEntry extends Table {
  public static RailVizStationDetailResEntry getRootAsRailVizStationDetailResEntry(ByteBuffer _bb) { return getRootAsRailVizStationDetailResEntry(_bb, new RailVizStationDetailResEntry()); }
  public static RailVizStationDetailResEntry getRootAsRailVizStationDetailResEntry(ByteBuffer _bb, RailVizStationDetailResEntry obj) { _bb.order(ByteOrder.LITTLE_ENDIAN); return (obj.__init(_bb.getInt(_bb.position()) + _bb.position(), _bb)); }
  public RailVizStationDetailResEntry __init(int _i, ByteBuffer _bb) { bb_pos = _i; bb = _bb; return this; }

  public String trainNum() { int o = __offset(4); return o != 0 ? __string(o + bb_pos) : null; }
  public ByteBuffer trainNumAsByteBuffer() { return __vector_as_bytebuffer(4, 1); }
  public String trainIdentifier() { int o = __offset(6); return o != 0 ? __string(o + bb_pos) : null; }
  public ByteBuffer trainIdentifierAsByteBuffer() { return __vector_as_bytebuffer(6, 1); }
  public int classz() { int o = __offset(8); return o != 0 ? bb.getInt(o + bb_pos) : -1; }
  public RailVizTrain train() { return train(new RailVizTrain()); }
  public RailVizTrain train(RailVizTrain obj) { int o = __offset(10); return o != 0 ? obj.__init(__indirect(o + bb_pos), bb) : null; }
  public String startendStationName() { int o = __offset(12); return o != 0 ? __string(o + bb_pos) : null; }
  public ByteBuffer startendStationNameAsByteBuffer() { return __vector_as_bytebuffer(12, 1); }
  public int startendStationId() { int o = __offset(14); return o != 0 ? bb.getInt(o + bb_pos) : 0; }
  public boolean outgoing() { int o = __offset(16); return o != 0 ? 0!=bb.get(o + bb_pos) : false; }

  public static int createRailVizStationDetailResEntry(FlatBufferBuilder builder,
      int train_num,
      int train_identifier,
      int classz,
      int train,
      int startend_station_name,
      int startend_station_id,
      boolean outgoing) {
    builder.startObject(7);
    RailVizStationDetailResEntry.addStartendStationId(builder, startend_station_id);
    RailVizStationDetailResEntry.addStartendStationName(builder, startend_station_name);
    RailVizStationDetailResEntry.addTrain(builder, train);
    RailVizStationDetailResEntry.addClassz(builder, classz);
    RailVizStationDetailResEntry.addTrainIdentifier(builder, train_identifier);
    RailVizStationDetailResEntry.addTrainNum(builder, train_num);
    RailVizStationDetailResEntry.addOutgoing(builder, outgoing);
    return RailVizStationDetailResEntry.endRailVizStationDetailResEntry(builder);
  }

  public static void startRailVizStationDetailResEntry(FlatBufferBuilder builder) { builder.startObject(7); }
  public static void addTrainNum(FlatBufferBuilder builder, int trainNumOffset) { builder.addOffset(0, trainNumOffset, 0); }
  public static void addTrainIdentifier(FlatBufferBuilder builder, int trainIdentifierOffset) { builder.addOffset(1, trainIdentifierOffset, 0); }
  public static void addClassz(FlatBufferBuilder builder, int classz) { builder.addInt(2, classz, -1); }
  public static void addTrain(FlatBufferBuilder builder, int trainOffset) { builder.addOffset(3, trainOffset, 0); }
  public static void addStartendStationName(FlatBufferBuilder builder, int startendStationNameOffset) { builder.addOffset(4, startendStationNameOffset, 0); }
  public static void addStartendStationId(FlatBufferBuilder builder, int startendStationId) { builder.addInt(5, startendStationId, 0); }
  public static void addOutgoing(FlatBufferBuilder builder, boolean outgoing) { builder.addBoolean(6, outgoing, false); }
  public static int endRailVizStationDetailResEntry(FlatBufferBuilder builder) {
    int o = builder.endObject();
    return o;
  }
};

