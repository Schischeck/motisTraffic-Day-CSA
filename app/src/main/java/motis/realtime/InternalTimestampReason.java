// automatically generated, do not modify

package motis.realtime;

public final class InternalTimestampReason {
  private InternalTimestampReason() { }
  public static final byte Schedule = 0;
  public static final byte Is = 1;
  public static final byte Forecast = 2;
  public static final byte Propagation = 3;
  public static final byte Repair = 4;

  private static final String[] names = { "Schedule", "Is", "Forecast", "Propagation", "Repair", };

  public static String name(int e) { return names[e]; }
};

