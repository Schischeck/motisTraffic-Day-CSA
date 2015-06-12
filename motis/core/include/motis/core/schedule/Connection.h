#ifndef TDCONNECTION_H
#define TDCONNECTION_H TDCONNECTION_H

#include <string>
#include <vector>
#include <tuple>

#include "motis/core/common/Array.h"
#include "motis/core/common/Pointer.h"
#include "motis/core/schedule/Time.h"

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

class ConnectionInfo
{
public:
  ConnectionInfo()
      : attributes(Array<int>::SizeType(0)),
        lineIdentifier(String::SizeType(0)),
        family(0),
        trainNr(0),
        service(0)
  {}

  bool operator < (ConnectionInfo const& o) const
  { return as_tuple() < o.as_tuple(); }

  bool operator == (ConnectionInfo const& o) const
  {
    return attributes == o.attributes &&
           lineIdentifier == o.lineIdentifier &&
           family == o.family &&
           trainNr == o.trainNr &&
           service == o.service;
  }

  std::tuple<Array<int>, String, uint32_t, uint32_t, uint32_t> as_tuple() const
  {
    return std::make_tuple(attributes,
                           lineIdentifier,
                           family,
                           trainNr,
                           service);
  }

  Array<int> attributes;
  String lineIdentifier;
  uint32_t family;
  uint32_t trainNr;
  uint32_t service;
};

class Connection
{
public:
  Connection() : conInfoId(0), price(0), dPlatform(0), aPlatform(0), clasz(0) {}

  bool operator==(Connection const& o) const
  { return clasz == o.clasz && price == o.price && conInfoId == o.conInfoId; }

  union {
    Pointer<ConnectionInfo const> conInfo;
    uint32_t conInfoId;
  };
  uint16_t price;
  uint16_t dPlatform, aPlatform;
  uint8_t clasz;
};

class LightConnection
{
public:
  LightConnection() = default;

  explicit LightConnection(Time dTime) : dTime(dTime) {}

  LightConnection(Time dTime, Time aTime,
                  Connection const* fullCon)
      : _fullCon(fullCon),
        dTime(dTime),
        aTime(aTime)
  {}

  unsigned travelTime() const { return aTime - dTime; }

  bool operator<(LightConnection const& o) const { return dTime < o.dTime; }

  bool operator==(LightConnection const& o) const
  { return dTime == o.dTime && aTime == o.aTime && *_fullCon == *o._fullCon; }

  Pointer<Connection const> _fullCon;
  Time dTime, aTime;

  enum : uint32_t { INVALID_CON_ID = 0xffffffff };
};

}  // namespace td

#endif //TDCONNECTION_H

