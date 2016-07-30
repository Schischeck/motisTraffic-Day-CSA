// automatically generated, do not modify

package motis.bikesharing;

public final class AvailabilityAggregator {
  private AvailabilityAggregator() { }
  public static final byte Average = 0;
  public static final byte Median = 1;
  public static final byte Minimum = 2;
  public static final byte Quantile90 = 3;
  public static final byte PercentReliable = 4;

  private static final String[] names = { "Average", "Median", "Minimum", "Quantile90", "PercentReliable", };

  public static String name(int e) { return names[e]; }
};

