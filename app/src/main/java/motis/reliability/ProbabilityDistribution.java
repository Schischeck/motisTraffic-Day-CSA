// automatically generated, do not modify

package motis.reliability;

import java.nio.*;
import java.lang.*;
import java.util.*;
import com.google.flatbuffers.*;

public final class ProbabilityDistribution extends Table {
  public static ProbabilityDistribution getRootAsProbabilityDistribution(ByteBuffer _bb) { return getRootAsProbabilityDistribution(_bb, new ProbabilityDistribution()); }
  public static ProbabilityDistribution getRootAsProbabilityDistribution(ByteBuffer _bb, ProbabilityDistribution obj) { _bb.order(ByteOrder.LITTLE_ENDIAN); return (obj.__init(_bb.getInt(_bb.position()) + _bb.position(), _bb)); }
  public ProbabilityDistribution __init(int _i, ByteBuffer _bb) { bb_pos = _i; bb = _bb; return this; }

  public long beginTime() { int o = __offset(4); return o != 0 ? bb.getLong(o + bb_pos) : 0; }
  public float distribution(int j) { int o = __offset(6); return o != 0 ? bb.getFloat(__vector(o) + j * 4) : 0; }
  public int distributionLength() { int o = __offset(6); return o != 0 ? __vector_len(o) : 0; }
  public ByteBuffer distributionAsByteBuffer() { return __vector_as_bytebuffer(6, 4); }
  public float sum() { int o = __offset(8); return o != 0 ? bb.getFloat(o + bb_pos) : 0; }

  public static int createProbabilityDistribution(FlatBufferBuilder builder,
      long begin_time,
      int distribution,
      float sum) {
    builder.startObject(3);
    ProbabilityDistribution.addBeginTime(builder, begin_time);
    ProbabilityDistribution.addSum(builder, sum);
    ProbabilityDistribution.addDistribution(builder, distribution);
    return ProbabilityDistribution.endProbabilityDistribution(builder);
  }

  public static void startProbabilityDistribution(FlatBufferBuilder builder) { builder.startObject(3); }
  public static void addBeginTime(FlatBufferBuilder builder, long beginTime) { builder.addLong(0, beginTime, 0); }
  public static void addDistribution(FlatBufferBuilder builder, int distributionOffset) { builder.addOffset(1, distributionOffset, 0); }
  public static int createDistributionVector(FlatBufferBuilder builder, float[] data) { builder.startVector(4, data.length, 4); for (int i = data.length - 1; i >= 0; i--) builder.addFloat(data[i]); return builder.endVector(); }
  public static void startDistributionVector(FlatBufferBuilder builder, int numElems) { builder.startVector(4, numElems, 4); }
  public static void addSum(FlatBufferBuilder builder, float sum) { builder.addFloat(2, sum, 0); }
  public static int endProbabilityDistribution(FlatBufferBuilder builder) {
    int o = builder.endObject();
    return o;
  }
};

