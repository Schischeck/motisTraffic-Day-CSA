// automatically generated, do not modify

package motis.reliability;

public final class RequestOptions {
  private RequestOptions() { }
  public static final byte NONE = 0;
  public static final byte RatingReq = 1;
  public static final byte ReliableSearchReq = 2;
  public static final byte LateConnectionReq = 3;
  public static final byte ConnectionTreeReq = 4;

  private static final String[] names = { "NONE", "RatingReq", "ReliableSearchReq", "LateConnectionReq", "ConnectionTreeReq", };

  public static String name(int e) { return names[e]; }
};

