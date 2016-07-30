// automatically generated, do not modify

package motis.ris;

import java.nio.*;
import java.lang.*;
import java.util.*;
import com.google.flatbuffers.*;

public final class ConnectionAssessment extends Table {
  public static ConnectionAssessment getRootAsConnectionAssessment(ByteBuffer _bb) { return getRootAsConnectionAssessment(_bb, new ConnectionAssessment()); }
  public static ConnectionAssessment getRootAsConnectionAssessment(ByteBuffer _bb, ConnectionAssessment obj) { _bb.order(ByteOrder.LITTLE_ENDIAN); return (obj.__init(_bb.getInt(_bb.position()) + _bb.position(), _bb)); }
  public ConnectionAssessment __init(int _i, ByteBuffer _bb) { bb_pos = _i; bb = _bb; return this; }

  public Event base() { return base(new Event()); }
  public Event base(Event obj) { int o = __offset(4); return o != 0 ? obj.__init(__indirect(o + bb_pos), bb) : null; }
  public int assessment() { int o = __offset(6); return o != 0 ? bb.getInt(o + bb_pos) : 0; }

  public static int createConnectionAssessment(FlatBufferBuilder builder,
      int base,
      int assessment) {
    builder.startObject(2);
    ConnectionAssessment.addAssessment(builder, assessment);
    ConnectionAssessment.addBase(builder, base);
    return ConnectionAssessment.endConnectionAssessment(builder);
  }

  public static void startConnectionAssessment(FlatBufferBuilder builder) { builder.startObject(2); }
  public static void addBase(FlatBufferBuilder builder, int baseOffset) { builder.addOffset(0, baseOffset, 0); }
  public static void addAssessment(FlatBufferBuilder builder, int assessment) { builder.addInt(1, assessment, 0); }
  public static int endConnectionAssessment(FlatBufferBuilder builder) {
    int o = builder.endObject();
    builder.required(o, 4);  // base
    return o;
  }
};

