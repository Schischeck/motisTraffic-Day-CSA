// automatically generated, do not modify

package motis.reliability;

import java.nio.*;
import java.lang.*;
import java.util.*;
import com.google.flatbuffers.*;

public final class AlternativeRating extends Table {
  public static AlternativeRating getRootAsAlternativeRating(ByteBuffer _bb) { return getRootAsAlternativeRating(_bb, new AlternativeRating()); }
  public static AlternativeRating getRootAsAlternativeRating(ByteBuffer _bb, AlternativeRating obj) { _bb.order(ByteOrder.LITTLE_ENDIAN); return (obj.__init(_bb.getInt(_bb.position()) + _bb.position(), _bb)); }
  public AlternativeRating __init(int _i, ByteBuffer _bb) { bb_pos = _i; bb = _bb; return this; }

  public ProbabilityDistribution departureDistribution() { return departureDistribution(new ProbabilityDistribution()); }
  public ProbabilityDistribution departureDistribution(ProbabilityDistribution obj) { int o = __offset(4); return o != 0 ? obj.__init(__indirect(o + bb_pos), bb) : null; }
  public ProbabilityDistribution arrivalDistribution() { return arrivalDistribution(new ProbabilityDistribution()); }
  public ProbabilityDistribution arrivalDistribution(ProbabilityDistribution obj) { int o = __offset(6); return o != 0 ? obj.__init(__indirect(o + bb_pos), bb) : null; }

  public static int createAlternativeRating(FlatBufferBuilder builder,
      int departure_distribution,
      int arrival_distribution) {
    builder.startObject(2);
    AlternativeRating.addArrivalDistribution(builder, arrival_distribution);
    AlternativeRating.addDepartureDistribution(builder, departure_distribution);
    return AlternativeRating.endAlternativeRating(builder);
  }

  public static void startAlternativeRating(FlatBufferBuilder builder) { builder.startObject(2); }
  public static void addDepartureDistribution(FlatBufferBuilder builder, int departureDistributionOffset) { builder.addOffset(0, departureDistributionOffset, 0); }
  public static void addArrivalDistribution(FlatBufferBuilder builder, int arrivalDistributionOffset) { builder.addOffset(1, arrivalDistributionOffset, 0); }
  public static int endAlternativeRating(FlatBufferBuilder builder) {
    int o = builder.endObject();
    return o;
  }
};

