// automatically generated, do not modify

package motis.routing;

import java.nio.*;
import java.lang.*;
import java.util.*;
import com.google.flatbuffers.*;

public final class AdditionalEdgeWrapper extends Table {
  public static AdditionalEdgeWrapper getRootAsAdditionalEdgeWrapper(ByteBuffer _bb) { return getRootAsAdditionalEdgeWrapper(_bb, new AdditionalEdgeWrapper()); }
  public static AdditionalEdgeWrapper getRootAsAdditionalEdgeWrapper(ByteBuffer _bb, AdditionalEdgeWrapper obj) { _bb.order(ByteOrder.LITTLE_ENDIAN); return (obj.__init(_bb.getInt(_bb.position()) + _bb.position(), _bb)); }
  public AdditionalEdgeWrapper __init(int _i, ByteBuffer _bb) { bb_pos = _i; bb = _bb; return this; }

  public byte additionalEdgeType() { int o = __offset(4); return o != 0 ? bb.get(o + bb_pos) : 0; }
  public Table additionalEdge(Table obj) { int o = __offset(6); return o != 0 ? __union(obj, o) : null; }

  public static int createAdditionalEdgeWrapper(FlatBufferBuilder builder,
      byte additional_edge_type,
      int additional_edge) {
    builder.startObject(2);
    AdditionalEdgeWrapper.addAdditionalEdge(builder, additional_edge);
    AdditionalEdgeWrapper.addAdditionalEdgeType(builder, additional_edge_type);
    return AdditionalEdgeWrapper.endAdditionalEdgeWrapper(builder);
  }

  public static void startAdditionalEdgeWrapper(FlatBufferBuilder builder) { builder.startObject(2); }
  public static void addAdditionalEdgeType(FlatBufferBuilder builder, byte additionalEdgeType) { builder.addByte(0, additionalEdgeType, 0); }
  public static void addAdditionalEdge(FlatBufferBuilder builder, int additionalEdgeOffset) { builder.addOffset(1, additionalEdgeOffset, 0); }
  public static int endAdditionalEdgeWrapper(FlatBufferBuilder builder) {
    int o = builder.endObject();
    return o;
  }
};

