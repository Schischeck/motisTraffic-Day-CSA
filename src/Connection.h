#ifndef TDCONNECTION_H
#define TDCONNECTION_H TDCONNECTION_H

#include <string>
#include <vector>
#include <limits>

#include "BitsetManager.h"
#include "TDTime.h"
#include "Array.h"
#include "serialization/Pointer.h"

namespace td
{

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

class InputConnection : public Connection, public ConnectionInfo
{
public:
  InputConnection() :
      trafficDayIndex(0),
      dTime(INVALID_TIME),
      aTime(INVALID_TIME)
  {}

  BitsetIndex trafficDayIndex;
  int dTime, aTime;
};

class LightConnection
{
public:
  LightConnection() = default;

  explicit LightConnection(Time dTime) : dTime(dTime) {}

  LightConnection(uint64_t lightConId,
                  Time dTime, Time aTime,
                  Connection const* fullCon)
      : _fullCon(fullCon),
        _conId(lightConId),
        _nextId(INVALID_CON_ID),
        dTime(dTime),
        aTime(aTime)
  {}

  unsigned travelTime() const { return aTime - dTime; }

  bool operator<(LightConnection const& o) const { return dTime < o.dTime; }

  bool operator==(LightConnection const& o) const
  { return dTime == o.dTime && aTime == o.aTime && *_fullCon == *o._fullCon; }

  Pointer<Connection const> _fullCon;
  union {
    Pointer<LightConnection const> _next;
    struct {
      uint32_t _conId;
      uint32_t _nextId;
    };
  };
  Time dTime, aTime;

  enum : uint32_t { INVALID_CON_ID = 0xffffffff };
};

}  // namespace td

#endif //TDCONNECTION_H

