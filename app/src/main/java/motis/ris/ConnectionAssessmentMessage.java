// automatically generated, do not modify

package motis.ris;

import java.nio.*;
import java.lang.*;
import java.util.*;
import com.google.flatbuffers.*;

public final class ConnectionAssessmentMessage extends Table {
  public static ConnectionAssessmentMessage getRootAsConnectionAssessmentMessage(ByteBuffer _bb) { return getRootAsConnectionAssessmentMessage(_bb, new ConnectionAssessmentMessage()); }
  public static ConnectionAssessmentMessage getRootAsConnectionAssessmentMessage(ByteBuffer _bb, ConnectionAssessmentMessage obj) { _bb.order(ByteOrder.LITTLE_ENDIAN); return (obj.__init(_bb.getInt(_bb.position()) + _bb.position(), _bb)); }
  public ConnectionAssessmentMessage __init(int _i, ByteBuffer _bb) { bb_pos = _i; bb = _bb; return this; }

  public Event from() { return from(new Event()); }
  public Event from(Event obj) { int o = __offset(4); return o != 0 ? obj.__init(__indirect(o + bb_pos), bb) : null; }
  public ConnectionAssessment to(int j) { return to(new ConnectionAssessment(), j); }
  public ConnectionAssessment to(ConnectionAssessment obj, int j) { int o = __offset(6); return o != 0 ? obj.__init(__indirect(__vector(o) + j * 4), bb) : null; }
  public int toLength() { int o = __offset(6); return o != 0 ? __vector_len(o) : 0; }

  public static int createConnectionAssessmentMessage(FlatBufferBuilder builder,
      int from,
      int to) {
    builder.startObject(2);
    ConnectionAssessmentMessage.addTo(builder, to);
    ConnectionAssessmentMessage.addFrom(builder, from);
    return ConnectionAssessmentMessage.endConnectionAssessmentMessage(builder);
  }

  public static void startConnectionAssessmentMessage(FlatBufferBuilder builder) { builder.startObject(2); }
  public static void addFrom(FlatBufferBuilder builder, int fromOffset) { builder.addOffset(0, fromOffset, 0); }
  public static void addTo(FlatBufferBuilder builder, int toOffset) { builder.addOffset(1, toOffset, 0); }
  public static int createToVector(FlatBufferBuilder builder, int[] data) { builder.startVector(4, data.length, 4); for (int i = data.length - 1; i >= 0; i--) builder.addOffset(data[i]); return builder.endVector(); }
  public static void startToVector(FlatBufferBuilder builder, int numElems) { builder.startVector(4, numElems, 4); }
  public static int endConnectionAssessmentMessage(FlatBufferBuilder builder) {
    int o = builder.endObject();
    builder.required(o, 4);  // from
    builder.required(o, 6);  // to
    return o;
  }
};

