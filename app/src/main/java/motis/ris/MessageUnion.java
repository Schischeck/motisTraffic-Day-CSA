// automatically generated, do not modify

package motis.ris;

public final class MessageUnion {
  private MessageUnion() { }
  public static final byte NONE = 0;
  public static final byte DelayMessage = 1;
  public static final byte CancelMessage = 2;
  public static final byte AdditionMessage = 3;
  public static final byte RerouteMessage = 4;
  public static final byte ConnectionDecisionMessage = 5;
  public static final byte ConnectionAssessmentMessage = 6;

  private static final String[] names = { "NONE", "DelayMessage", "CancelMessage", "AdditionMessage", "RerouteMessage", "ConnectionDecisionMessage", "ConnectionAssessmentMessage", };

  public static String name(int e) { return names[e]; }
};

