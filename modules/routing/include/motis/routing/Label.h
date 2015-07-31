#ifndef TD_LABEL_H_
#define TD_LABEL_H_

#include <cmath>
#include <cstring>
#include <cmath>
#include <memory>

#include "motis/core/schedule/Edges.h"
#include "motis/core/schedule/Connection.h"
#include "motis/routing/LowerBounds.h"
#include "motis/routing/Filters.h"
#include "motis/routing/MemoryManager.h"

#define MINUTELY_WAGE 8
#define TRANSFER_WAGE 0
#define MAX_LABELS 100000000
#define MAX_LABELS_WITH_MARGIN (MAX_LABELS + 1000)
#define TD_MAX_REGIONAL_TRAIN_TICKET_PRICE (4200u)

namespace td {

class Label {
public:
  enum {
    TD_LOCAL_TRANSPORT_PRICE,
    TD_REGIONAL_TRAIN_PRICE,
    TD_IC_PRICE,
    TD_ICE_PRICE,
    ADDITIONAL_PRICE
  };

  Label() = default;

  Label(Node const* node,
        Label* pred,
        Time now,
        LowerBounds& lowerBounds)
      : _pred(pred),
        _node(node),
        _connection(nullptr),
        _start(pred != nullptr ? pred->_start : now),
        _now(now),
        _dominated(false),
        _targetSlot(pred == nullptr ? 0 : pred->_targetSlot)
  {
    _travelTime[0] = _now - _start;
    _travelTime[1] = _travelTime[0];
    _travelTime[1] += lowerBounds.travelTime.getDistance(node->_id);

    _transfers[0] = pred != nullptr ? pred->_transfers[0] : 0;
    _transfers[1] = _transfers[0];
    _transfers[1] += lowerBounds.transfers.getDistance(node->_id);

    _totalPrice[0] = pred != nullptr ? pred->_totalPrice[0] : 0;

    if (pred != nullptr)
      std::memcpy(_prices, pred->_prices, sizeof(_prices));
    else
      std::memset(_prices, 0, sizeof(_prices));

    setTotalPriceLowerBound(lowerBounds.price.getDistance(node->_id));
  }

  Label* createLabel(
      Edge const& edge,
      LowerBounds& lowerBounds,
      MemoryManager<Label>& labelStore) {
    auto nNode = edge.getDestination()->_id;

    uint32_t transfersLB = lowerBounds.transfers.getDistance(nNode);
    if (transfersLB == std::numeric_limits<uint32_t>::max())
      return nullptr;

    EdgeCost ec = edge.getEdgeCost(_now, _connection);

    unsigned nTravelTime = _travelTime[0] + ec.time;
    unsigned nTravelTimeLB = nTravelTime;
    nTravelTimeLB += lowerBounds.travelTime.getDistance(nNode);

    unsigned nTransfers = _transfers[0] + (ec.transfer ? 1 : 0);
    unsigned nTransfersLB = nTransfers + transfersLB;

    if(ec == NO_EDGE ||
       (_pred != nullptr && edge.getDestination() == _pred->_node) ||
       isFilteredTravelTime(nTravelTimeLB) ||
       isFilteredTransfers(nTransfersLB) ||
       isFilteredWaitingTime(ec.connection, _travelTime[0], nTravelTime))
      return nullptr;

    Label* l = new (labelStore.create()) Label(*this);
    l->_travelTime[0] = nTravelTime;
    l->_travelTime[1] = nTravelTimeLB;
    l->_transfers[0] = nTransfers;
    l->_transfers[1] = nTransfersLB;
    l->addPriceOfConnection(ec, edge._m._type == Edge::Type::MUMO_EDGE);
    l->setTotalPriceLowerBound(lowerBounds.price.getDistance(nNode));
    l->_pred = this;
    l->_start = _start;
    l->_now = _now + ec.time;
    l->_node = edge.getDestination();
    l->_connection = ec.connection;

    return l;
  }

  bool dominates(Label const& o, bool lowerBound) const
  {
    /* --- CHECK MAY DOMINATE --- */
    if (_start < o._start || _now > o._now)
      return false;

    int index = lowerBound ? 1 : 0;
    bool couldDominate = false;


    /* --- TRANSFERS --- */
    if (_transfers[index] > o._transfers[index])
      return false;
    couldDominate = couldDominate || _transfers[index] < o._transfers[index];


    /* --- TRAVEL TIME --- */
    if (_travelTime[index] > o._travelTime[index])
      return false;
    couldDominate = couldDominate || _travelTime[index] < o._travelTime[index];


    /* --- PRICE --- */
    unsigned myPrice = _totalPrice[index];
    unsigned oPrice = o._totalPrice[index];

    if (lowerBound)
    {
      myPrice += _travelTime[1] * MINUTELY_WAGE + _transfers[1] * TRANSFER_WAGE;
      oPrice += o._travelTime[1] * MINUTELY_WAGE + o._transfers[1] * TRANSFER_WAGE;
    }
    else
    {
      myPrice += o._now - _start * MINUTELY_WAGE + _transfers[0] * TRANSFER_WAGE;
      oPrice += o._now - o._start * MINUTELY_WAGE + o._transfers[0] * TRANSFER_WAGE;
    }

#ifdef PRICE_TOLERANCE
    unsigned tolerance = std::floor(0.01 * std::min(myPrice, oPrice));
    if (std::labs(myPrice - oPrice) > tolerance)
    {
#endif
      if (myPrice > oPrice)
        return false;

      couldDominate = couldDominate || myPrice < oPrice;
#ifdef PRICE_TOLERANCE
    }
#endif

    /* --- ALL CRITERIA --- */
    return true;
  }

  bool dominatesHard(Label const& o) const {
    bool couldDominate = false;

    /* --- TRANSFERS --- */
    if (_transfers[0] > o._transfers[0])
      return false;
    couldDominate = couldDominate || _transfers[0] < o._transfers[0];


    /* --- TRAVEL TIME --- */
    if (_travelTime[0] > o._travelTime[0])
      return false;
    couldDominate = couldDominate || _travelTime[0] < o._travelTime[0];


    /* --- PRICE --- */
    unsigned myPrice = getPriceWithWages(false);
    unsigned oPrice = o.getPriceWithWages(false);
    if (myPrice > oPrice)
      return false;
    couldDominate = couldDominate || myPrice < oPrice;

    return couldDominate || _start >= o._start;
  }

  bool operator<(Label const& o) const
  {
    if (_travelTime[1] != o._travelTime[1])
      return _travelTime[1] < o._travelTime[1];

    if (_transfers[1] != o._transfers[1])
      return _transfers[1] < o._transfers[1];

    unsigned myPriceHeuristic = getPriceWithWages(true);
    unsigned oPriceHeuristic = o.getPriceWithWages(true);
    return myPriceHeuristic < oPriceHeuristic || _start > o._start;
  }

  unsigned getTravelTimePrice() const { return _travelTime[0] * MINUTELY_WAGE; }

  int getPriceWithWages(bool lowerBound) const
  {
    return _totalPrice[lowerBound ? 1 : 0] +
           _travelTime[lowerBound ? 1 : 0] * MINUTELY_WAGE +
           _transfers[lowerBound ? 1 : 0] * TRANSFER_WAGE;
  }

  void addPriceOfConnection(EdgeCost const& ec, bool mumoEdge)
  {
    if (ec.connection != nullptr)
    {
      Connection const* con = ec.connection->_fullCon;
      switch(con->clasz)
      {
        case TD_ICE:
          if (_prices[TD_ICE_PRICE] == 0)
          {
            if (_prices[TD_IC_PRICE] != 0)
              _prices[TD_ICE_PRICE] += 100 + con->price;
            else
              _prices[TD_ICE_PRICE] += 700 + con->price;
          }
          else
            _prices[TD_ICE_PRICE] += con->price;

          break;

        case TD_IC:
          if (_prices[TD_IC_PRICE] == 0)
          {
            if (_prices[TD_ICE_PRICE] != 0)
              _prices[TD_IC_PRICE] += con->price;
            else
              _prices[TD_IC_PRICE] += 600 + con->price;
          }
          else
            _prices[TD_IC_PRICE] += 600 + con->price;

          break;

        case TD_RE:
        case TD_RB:
        case TD_S:
          _prices[TD_REGIONAL_TRAIN_PRICE] += con->price;
          if (_prices[TD_REGIONAL_TRAIN_PRICE] > TD_MAX_REGIONAL_TRAIN_TICKET_PRICE)
            _prices[TD_REGIONAL_TRAIN_PRICE] = TD_MAX_REGIONAL_TRAIN_TICKET_PRICE;

          break;

        case TD_N:
        case TD_U:
        case TD_STR:
        case TD_BUS:
        case TD_X:
          _prices[TD_LOCAL_TRANSPORT_PRICE] += con->price;
      }
    }
    else
    {
      _prices[4] += ec[2];
      if (mumoEdge)
        setSlot(false, ec.slot);
    }

    unsigned priceSum = _prices[0] + _prices[1] + _prices[2] + _prices[3];
    auto trainPrice = std::min(priceSum, 14000u);

    _totalPrice[0] = trainPrice + _prices[4];
  }

  void setTotalPriceLowerBound(uint32_t lowerBound)
  {
    assert(lowerBound <= std::numeric_limits<uint16_t>::max());
    _totalPrice[1] = _prices[ADDITIONAL_PRICE] +
                     std::min(14000u,
                              _prices[TD_LOCAL_TRANSPORT_PRICE] +
                              _prices[TD_IC_PRICE] +
                              _prices[TD_ICE_PRICE] +
                              std::min(_prices[TD_REGIONAL_TRAIN_PRICE] + lowerBound,
                                       TD_MAX_REGIONAL_TRAIN_TICKET_PRICE));
  }

  void setSlot(bool start, int slot)
  {
    if (!start)
      _targetSlot = slot;
    else if (_pred == nullptr)
      _travelTime[0] = slot;
    else
      _pred->setSlot(start, slot);
  }

  int getSlot(bool start) const
  {
    if (!start)
      return _targetSlot;
    else
    {
      Label const* current = this;
      Label const* pred = _pred;
      while (pred != nullptr)
      {
        auto old_pred = pred;
        pred = current->_pred;
        current = old_pred;
      }
      return current->_travelTime[0];
    }
  }

  Label* _pred;
  Node const* _node;
  LightConnection const* _connection;
  uint16_t _travelTime[2];
  uint16_t _totalPrice[2];
  uint16_t _prices[5];
  Time _start, _now;
  uint8_t _transfers[2];
  bool _dominated;
  uint8_t _targetSlot;
};

}  // namespace td

#endif  // TD_LABEL_H_
