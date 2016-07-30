// automatically generated, do not modify

package motis.lookup;

import java.nio.*;
import java.lang.*;
import java.util.*;
import com.google.flatbuffers.*;

public final class LookupIdTrainRequest extends Table {
  public static LookupIdTrainRequest getRootAsLookupIdTrainRequest(ByteBuffer _bb) { return getRootAsLookupIdTrainRequest(_bb, new LookupIdTrainRequest()); }
  public static LookupIdTrainRequest getRootAsLookupIdTrainRequest(ByteBuffer _bb, LookupIdTrainRequest obj) { _bb.order(ByteOrder.LITTLE_ENDIAN); return (obj.__init(_bb.getInt(_bb.position()) + _bb.position(), _bb)); }
  public LookupIdTrainRequest __init(int _i, ByteBuffer _bb) { bb_pos = _i; bb = _bb; return this; }

  public IdEvent idEvent() { return idEvent(new IdEvent()); }
  public IdEvent idEvent(IdEvent obj) { int o = __offset(4); return o != 0 ? obj.__init(__indirect(o + bb_pos), bb) : null; }

  public static int createLookupIdTrainRequest(FlatBufferBuilder builder,
      int id_event) {
    builder.startObject(1);
    LookupIdTrainRequest.addIdEvent(builder, id_event);
    return LookupIdTrainRequest.endLookupIdTrainRequest(builder);
  }

  public static void startLookupIdTrainRequest(FlatBufferBuilder builder) { builder.startObject(1); }
  public static void addIdEvent(FlatBufferBuilder builder, int idEventOffset) { builder.addOffset(0, idEventOffset, 0); }
  public static int endLookupIdTrainRequest(FlatBufferBuilder builder) {
    int o = builder.endObject();
    return o;
  }
};

