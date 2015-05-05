#ifndef TD_PRICES_H_
#define TD_PRICES_H_

// Refers to the price of "Schoenes Wochenende Ticket"
#define TD_MAX_REGIONAL_TRAIN_TICKET_PRICE (4200u)

namespace td
{

enum {
  TD_ICE = 0,
  TD_IC  = 1,
  TD_N   = 2,
  TD_RE  = 3,
  TD_RB  = 4,
  TD_S   = 5,
  TD_U   = 6,
  TD_STR = 7,
  TD_BUS = 8,
  TD_X   = 9
};

class Prices {
  public:
    static int getPricePerKm(int clasz)
    {
      switch(clasz)
      {
        case TD_ICE:
          return 22;

        case TD_N:
        case TD_IC:
        case TD_X:
          return 18;

        case TD_RE:
        case TD_RB:
        case TD_S:
        case TD_U:
        case TD_STR:
        case TD_BUS:
          return 15;

        default:
          return 0;
      }
    }
};

}  // namespace td

#endif  // TD_PRICES_H_
