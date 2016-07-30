// automatically generated, do not modify

package motis.reliability;

import java.nio.*;
import java.lang.*;
import java.util.*;
import com.google.flatbuffers.*;

public final class RatingElement extends Table {
  public static RatingElement getRootAsRatingElement(ByteBuffer _bb) { return getRootAsRatingElement(_bb, new RatingElement()); }
  public static RatingElement getRootAsRatingElement(ByteBuffer _bb, RatingElement obj) { _bb.order(ByteOrder.LITTLE_ENDIAN); return (obj.__init(_bb.getInt(_bb.position()) + _bb.position(), _bb)); }
  public RatingElement __init(int _i, ByteBuffer _bb) { bb_pos = _i; bb = _bb; return this; }

  public Range range() { return range(new Range()); }
  public Range range(Range obj) { int o = __offset(4); return o != 0 ? obj.__init(o + bb_pos, bb) : null; }
  public ProbabilityDistribution depDistribution() { return depDistribution(new ProbabilityDistribution()); }
  public ProbabilityDistribution depDistribution(ProbabilityDistribution obj) { int o = __offset(6); return o != 0 ? obj.__init(__indirect(o + bb_pos), bb) : null; }
  public ProbabilityDistribution arrDistribution() { return arrDistribution(new ProbabilityDistribution()); }
  public ProbabilityDistribution arrDistribution(ProbabilityDistribution obj) { int o = __offset(8); return o != 0 ? obj.__init(__indirect(o + bb_pos), bb) : null; }

  public static void startRatingElement(FlatBufferBuilder builder) { builder.startObject(3); }
  public static void addRange(FlatBufferBuilder builder, int rangeOffset) { builder.addStruct(0, rangeOffset, 0); }
  public static void addDepDistribution(FlatBufferBuilder builder, int depDistributionOffset) { builder.addOffset(1, depDistributionOffset, 0); }
  public static void addArrDistribution(FlatBufferBuilder builder, int arrDistributionOffset) { builder.addOffset(2, arrDistributionOffset, 0); }
  public static int endRatingElement(FlatBufferBuilder builder) {
    int o = builder.endObject();
    return o;
  }
};

