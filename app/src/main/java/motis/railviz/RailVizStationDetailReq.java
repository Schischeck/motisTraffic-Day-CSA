// automatically generated, do not modify

package motis.railviz;

import java.nio.*;
import java.lang.*;
import java.util.*;
import com.google.flatbuffers.*;

public final class RailVizStationDetailReq extends Table {
  public static RailVizStationDetailReq getRootAsRailVizStationDetailReq(ByteBuffer _bb) { return getRootAsRailVizStationDetailReq(_bb, new RailVizStationDetailReq()); }
  public static RailVizStationDetailReq getRootAsRailVizStationDetailReq(ByteBuffer _bb, RailVizStationDetailReq obj) { _bb.order(ByteOrder.LITTLE_ENDIAN); return (obj.__init(_bb.getInt(_bb.position()) + _bb.position(), _bb)); }
  public RailVizStationDetailReq __init(int _i, ByteBuffer _bb) { bb_pos = _i; bb = _bb; return this; }

  public long stationIndex() { int o = __offset(4); return o != 0 ? (long)bb.getInt(o + bb_pos) & 0xFFFFFFFFL : 0; }

  public static int createRailVizStationDetailReq(FlatBufferBuilder builder,
      long station_index) {
    builder.startObject(1);
    RailVizStationDetailReq.addStationIndex(builder, station_index);
    return RailVizStationDetailReq.endRailVizStationDetailReq(builder);
  }

  public static void startRailVizStationDetailReq(FlatBufferBuilder builder) { builder.startObject(1); }
  public static void addStationIndex(FlatBufferBuilder builder, long stationIndex) { builder.addInt(0, (int)(stationIndex & 0xFFFFFFFFL), 0); }
  public static int endRailVizStationDetailReq(FlatBufferBuilder builder) {
    int o = builder.endObject();
    return o;
  }
};

