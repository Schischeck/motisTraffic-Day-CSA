///////////////////////////////////////////////////////////////////////////////
// task: conversion of a .Services.txt and a .Bitfields.txt file into a      //
//       format storing train routes                                         //
//                                                                           //
// usage: services2routes [Dataset prefix e.g. "July2007"]                   //
//                                                                           //
// author: Yann Disser (Skratchie@gmx.de)                                    //
//                                                                           //
// creation date: 31.7.2007                                                  //
///////////////////////////////////////////////////////////////////////////////

#include <string>
#include <vector>
#include <iostream>
#include <cassert>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <stdexcept>

#include "serialization/Files.h"
#include "BitsetManager.h"
#include "Logging.h"

using namespace std;

namespace td {

using namespace logging;

struct TrainStationInfo
{
  int aTime;
  int aPlatform;
  int dTime;
  int dPlatform;
  int trafficDayIndex;
  int transferClass;
  int linienzuordnung;
  vector<int> attributes;
  int trainNr;
  string evaNr;
  string lineIdentifier;

  void write(ostream& out, bool first, bool last) const
  {
    if(!first)
      out << "a" << aTime << " " << aPlatform;
    if(!first && !last)
      out << " ";
    if(!last)
    {
      out << "d" << dTime << " " << dPlatform
          << " " << trafficDayIndex
          << " " << transferClass << " " << linienzuordnung << " "
          << attributes.size();
      for(unsigned int i = 0; i < attributes.size(); ++i)
        out << " " << attributes[i];
      out << " " << trainNr << "%" << evaNr << "|" << lineIdentifier << "|";
    }
  }
};

struct TrainInfo
{
  int trainIndex;
  vector<TrainStationInfo> stations;

  void write(ostream& out) const
  {
    out << "s" << trainIndex << " ";
    for(unsigned int i = 0; i < stations.size(); ++i)
    {
      stations[i].write(out, i == 0, i == stations.size() - 1);
      if(i < stations.size() - 1)
        out << ", ";
    }
  }
};

struct RouteStationInfo
{
  int location;
  bool skipArrival;
  bool skipDeparture;
  int family;

  bool operator==(const RouteStationInfo& o) const
  { return o.location == location && o.skipArrival == skipArrival &&
           o.skipDeparture == skipDeparture && o.family == family; }

  bool operator<(const RouteStationInfo& o) const
  {
    if(location != o.location)
      return location < o.location;
    if(skipArrival != o.skipArrival)
      return skipArrival < o.skipArrival;
    if(skipDeparture != o.skipDeparture)
      return skipDeparture < o.skipDeparture;
    if(family != o.family)
      return family < o.family;
    else
      return false;
  }

  void write(ostream& out, bool first, bool last) const
  {
    out << "l" << location;
    if(!first)
      out << " a" << skipArrival;
    if(!last)
      out << " d" << skipDeparture << " " << family;
  }
};

struct RouteInfo
{
  vector<RouteStationInfo> stations;

  bool operator==(const RouteInfo& o) const
  { return stations == o.stations; }

  bool operator<(const RouteInfo& o) const
  {
    unsigned int n = stations.size(), n2 = o.stations.size();
    if(n != n2)
      return n < n2;
    for(unsigned int i = 0; i < n; ++i)
      if(!(stations[i] == o.stations[i]))
        return stations[i] < o.stations[i];
    return false;
  }

  void write(ostream& out) const
  {
    for(unsigned int i = 0; i < stations.size(); ++i)
    {
      stations[i].write(out, i == 0, i == stations.size() - 1);
      if(i < stations.size() - 1)
        out << ", ";
    }
  }
};

struct ServiceStationInfo
{
  int location;
  bool skipArrival;
  int aTime;
  int aPlatform;
  bool skipDeparture;
  int dTime;
  int dPlatform;
  int trafficDayIndex;
  int transferClass;
  int linienzuordnung;
  int family;
  vector<int> attributes;
  int trainNr;
  string evaNr;
  string lineIdentifier;
};

struct ServiceInfo
{
  vector<ServiceStationInfo> stations;

  bool operator<(const ServiceInfo& o) const
  { return stations[0].dTime < o.stations[0].dTime; }

   /**
   * Returns a vector that stores how many night/day shifts have ocuured up
   * to station i. Needed for splitting and merging.
   */
  vector<int> getTrafficDayShifts()
  {
    //the last station has no traffic day info
    vector<int> ret(stations.size() - 1, 0);

    int shifts = 0;
    for(unsigned int i = 1; i < stations.size() - 1; ++i)
    {
      if(stations[i].dTime < stations[i - 1].dTime)
        ++shifts;
      ret[i] = shifts;
    }

    return ret;
  }

  /**
   * Splits itself into several ServiceInfos which are consistent in their
   * traffic days.
   * @return vector with the new ServiceInfos
   */
  vector<ServiceInfo> split(BitsetManager& bm)
  {
    //vector to return
    vector<ServiceInfo> vsi;

    //get night/day shifts so that we can shift all trafficDayIndices before
    //comparing them
    vector<int> shifts = getTrafficDayShifts();

    //check for each pair of adjacent stations whether their traffic days differ
    for(unsigned int i = 0; i < stations.size() - 2; ++i)
    {
      //compare the two shifted masks ignoring the last N bits where
      //N is the difference in shifts
      //after shifting we are unsure about the bits which were shifted in
      //so we cannot compare them
      if(!bm.equals(bm.shiftL(stations[i].trafficDayIndex, shifts[i]),
                    bm.shiftL(stations[i + 1].trafficDayIndex, shifts[i + 1]),
                    shifts[i + 1] - shifts[i]))
      {
        int commonTrafficDays =
          bm.commonBitset(bm.shiftL(stations[i].trafficDayIndex, shifts[i]),
                     bm.shiftL(stations[i + 1].trafficDayIndex, shifts[i + 1]));
        //si2 will contain the incompatible traffic days of the first part
        ServiceInfo si2;
        for(unsigned int j = 0; j <= i + 1; ++j)
        {
          si2.stations.push_back(stations[j]);
          if(j < i + 1)
          {
            si2.stations[j].trafficDayIndex =
              bm.without(si2.stations[j].trafficDayIndex,
                bm.shiftR(commonTrafficDays, shifts[j]));
          }
        }
        //si3 will contain the incompatible traffic days of the second part
        ServiceInfo si3;
        for(unsigned int j = i + 1; j < stations.size(); ++j)
        {
          si3.stations.push_back(stations[j]);
          if(j < stations.size() - 1)
          {
            si3.stations[j - i - 1].trafficDayIndex =
            bm.without(si3.stations[j - i - 1].trafficDayIndex,
                bm.shiftR(commonTrafficDays, shifts[j]));
          }
        }

        //the first part cannot have anymore conflicts - add it if non-trivial
        int trafficDayIndex = si2.stations[0].trafficDayIndex;
        if(trafficDayIndex != EMPTY_BITSET)
          vsi.push_back(si2);
        //the second part may have further conflicts -> recursion
        vector<ServiceInfo> vsi2 = si3.split(bm);
        vsi.insert(vsi.end(), vsi2.begin(), vsi2.end());

        //remove all traffic days which are not in common
        for(unsigned int j = 0; j < stations.size() - 1; ++j)
          stations[j].trafficDayIndex =
            bm.commonBitset(stations[j].trafficDayIndex,
                bm.shiftR(commonTrafficDays, shifts[j]));

        //the common part may also have further conflicts ->recursion
        if(stations[0].trafficDayIndex != EMPTY_BITSET)
        {
          vector<ServiceInfo> vsi3 = this->split(bm);
          vsi.insert(vsi.end(), vsi3.begin(), vsi3.end());
        }

        return vsi;
      }
    }

    //no more splitting needed? -> recursion anchor
    if(stations[0].trafficDayIndex != EMPTY_BITSET)
      vsi.push_back(*this);

    return vsi;
  }


  /**
   * Sets the traffic days of all stations to index
   */
  void setTrafficDays(int index, BitsetManager& bm)
  {
    vector<int> shifts = getTrafficDayShifts();
    for(unsigned int i = 0; i < stations.size() - 1; ++i)
      stations[i].trafficDayIndex = bm.shiftR(index, shifts[i]);
  }

  /**
   * Removes the traffic days in index from all stations
   */
  ServiceInfo removeTrafficDays(int index, BitsetManager& bm)
  {
    vector<int> shifts = getTrafficDayShifts();
    ServiceInfo ret = *this;
    for(unsigned int i = 0; i < stations.size() - 1; ++i)
      ret.stations[i].trafficDayIndex =
       bm.without(ret.stations[i].trafficDayIndex, bm.shiftR(index, shifts[i]));
    return ret;
  }

  /**
   * merges the two ServiceInfos and returns the number of resulting ServiceInfo
   * objects.
   * If two objects are to be connected that have different traffic
   * days, the common set of traffic days is used for the merged ServiceInfo.
   * Up to two more ServiceInfos have to be created, that contain the remaining
   * traffic days
   * @param The ServiceInfo to merge with this
   * @param If 3 is returned this parameter contains a new Service
   * @return Number of Services after merging.
   **/
  int merge(/*in/out*/ServiceInfo& o, /*out*/ServiceInfo& o2, BitsetManager& bm)
  {
    int size = stations.size();
    assert(size > 1);
    //the two services have to be connectible
    if(stations[size - 1].location != o.stations[0].location)
      return 2;

    int shifts = 0;
    //day/night shift?
    if(stations[size - 2].dTime > o.stations[0].dTime)
      shifts = 1;

    int index1 = stations[size - 2].trafficDayIndex;
    int index2 = bm.shiftL(o.stations[0].trafficDayIndex, shifts);

    //ignore shifted bits
    if(bm.equals(index1, index2, shifts))
    {
      int aTime = stations[size - 1].aTime;
      bool skipArrival = stations[size - 1].skipArrival;
      stations.pop_back();

      stations.insert(stations.end(), o.stations.begin(), o.stations.end());
      stations[size - 1].aTime = aTime;
      stations[size - 1].skipArrival = skipArrival;

      return 1;
    }
    else
    {
      int common = bm.commonBitset(index1, index2);
      //is the common set empty? (disregarding shifted bits)
      if(bm.equals(common, EMPTY_BITSET, shifts))
        return 2;

      vector<int> shiftsVec = getTrafficDayShifts();

      o2 = o.removeTrafficDays(bm.shiftR(common, shifts), bm);

      ServiceInfo o3 =
        removeTrafficDays(bm.shiftL(common, shiftsVec[size - 2]), bm);

      setTrafficDays(bm.shiftL(common, shiftsVec[size - 2]), bm);
      o.setTrafficDays(bm.shiftR(common, shifts), bm);

      int aTime = stations[size - 1].aTime;
      bool skipArrival = stations[size - 1].skipArrival;
      stations.pop_back();

      stations.insert(stations.end(), o.stations.begin(), o.stations.end());
      stations[size - 1].aTime = aTime;
      stations[size - 1].skipArrival = skipArrival;

      int ret = 1;

      //handle o2
      if(!bm.equals(o2.stations[0].trafficDayIndex, EMPTY_BITSET, shifts))
      {
        o = o2;
        ret++;
      }

      //handle o3
      if(!bm.equals(o3.stations[0].trafficDayIndex, EMPTY_BITSET,
                   shiftsVec[size - 2]))
      {
        if(ret == 2)
          o2 = o3;
        else
          o = o3;
        ret++;
      }

      return ret;
    }
  }
};

struct CompressedServiceInfo
{
  vector<ServiceStationInfo> stations;
  vector<int> times, bitfields;
  int index;
};

void readStation(istream& in, ServiceStationInfo& ssi, bool firstOrLast)
{
  char c;
  in >> c;

  assert(c == 'l');
  in >> ssi.location;

  in >> c;
  if(!firstOrLast || c == 'a')
  {
    assert(c == 'a');
    in >> ssi.skipArrival >> ssi.aTime;
  }
  else
    assert(c == 'd');

  if(!firstOrLast)
  {
    in >> c;
    assert(c == 'd');
  }

  if(c == 'd')
  {
    in >> ssi.skipDeparture >> ssi.dTime >> ssi.trafficDayIndex
       >> ssi.transferClass >> ssi.linienzuordnung >> ssi.family;
    unsigned int n;
    in >> n;
    ssi.attributes.resize(n);
    for(unsigned int i = 0; i < n; ++i)
      in >> ssi.attributes[i];
    in >> ssi.trainNr;
    in >> c;
    assert(c == '%');
    getline(in, ssi.evaNr, '|');

    string trainName;
    getline(in, trainName, '|');

    getline(in, ssi.lineIdentifier, '|');

    in >> ssi.dPlatform >> ssi.aPlatform;
  }
}

int readService(istream& in, vector<vector<ServiceInfo> >& services,
                vector<CompressedServiceInfo>& compressedServices)
{
  char c;
  in >> c;
  if(in.eof())
    return -1;
  assert(c == 's');
  int index;
  in >> index;

  unsigned int n;
  in >> n;
  //compressed service?
  in >> c;
  if(c == 't')
  {
    int ntimes, nBitfields;
    in >> ntimes;

    CompressedServiceInfo csi;
    csi.index = index;

    csi.times.resize(ntimes);
    for(int i = 0; i < ntimes; ++i)
      in >> csi.times[i];

    in >> c;
    assert(c == 'b');
    in >> nBitfields;
    assert(nBitfields == ntimes);
    csi.bitfields.resize(nBitfields);
    for(int i = 0; i < nBitfields; ++i)
      in >> csi.bitfields[i];

    csi.stations.resize(n);
    for(unsigned int i = 0; i < n; ++i)
      readStation(in, csi.stations[i], i == 0 || i == n - 1);

    compressedServices.push_back(csi);

    return -2;
  }
  else
  {
    in.putback(c);

    ServiceInfo si;
    si.stations.resize(n);
    for(unsigned int i = 0; i < n; ++i)
      readStation(in, si.stations[i], i == 0 || i == n - 1);

    while(index >= (int)services.size())
      services.push_back(vector<ServiceInfo>());
    services[index].push_back(si);

    return index;
  }
}

int mergeServices(vector<ServiceInfo>& vsi, BitsetManager& bm)
{
  int mergeCount = 0;

  for(unsigned int i = 0; i < vsi.size(); ++i)
    for(unsigned int j = 0; j < vsi.size(); ++j)
    {
      if(j == i)
        continue;
      int num;
      ServiceInfo temp;
      if((num = vsi[i].merge(vsi[j], temp, bm)) != 2)
      {
        ++mergeCount;
        if(num == 1)
        {
          if(j < i)
            --i;
          vsi.erase(vsi.begin() + j);
        }
        else if(num == 3)
          vsi.push_back(temp);

        j = -1;
      }
    }

  return mergeCount;
}

void extractRouteInfo(const ServiceInfo& si, RouteInfo& ri, TrainInfo& ti)
{
  for(unsigned int i = 0; i < si.stations.size(); ++i)
  {
    RouteStationInfo rsi;
    rsi.location = si.stations[i].location;
    rsi.skipArrival = si.stations[i].skipArrival;
    rsi.skipDeparture = si.stations[i].skipDeparture;
    rsi.family = si.stations[i].family;
    ri.stations.push_back(rsi);

    TrainStationInfo tsi;
    tsi.aTime = si.stations[i].aTime;
    tsi.aPlatform = (i != 0) ? si.stations[i - 1].aPlatform : 0;
    tsi.dTime = si.stations[i].dTime;
    tsi.dPlatform = si.stations[i].dPlatform;
    tsi.trafficDayIndex = si.stations[i].trafficDayIndex;
    tsi.transferClass = si.stations[i].transferClass;
    tsi.linienzuordnung = si.stations[i].linienzuordnung;
    tsi.attributes = si.stations[i].attributes;
    tsi.trainNr = si.stations[i].trainNr;
    tsi.evaNr = si.stations[i].evaNr;
    tsi.lineIdentifier = si.stations[i].lineIdentifier;
    ti.stations.push_back(tsi);
  }
}

const char* BITFIELD_FILE = ".Bitfields.txt";
const char* NEW_BITFIELD_FILE = ".Route.Bitfields.txt";

void services2Routes(std::string const& prefix)
{
  /////////////////////////////////////////////////////////////////////////////
  //read bitfields
  LOG(info) << "reading bitfields";
  ifstream bin;
  bin.exceptions(std::ios_base::failbit);
  bin.open((prefix + BITFIELD_FILE).c_str());
  BitsetManager bsMan(bin);
  bin.close();
  LOG(info) << "done";

  ifstream in;
  in.open((prefix + SERVICES_FILE).c_str());

  // The events of a train in the services file does not necessarily
  // appear in consecutive lines and can be separated.
  // Therefore, separated parts of the same train have to be merged
  // to one complete train.
  // The vector 'services' contains all trains in the services file.
  // Each element is a vector containing all parts of the same train.
  vector<vector<ServiceInfo>> services(100000);
  vector<CompressedServiceInfo> compressedServices;

  /////////////////////////////////////////////////////////////////////////////
  //read services
  LOG(debug) << "reading services";
  unsigned int lastI = 0;
  while(!in.eof() && in.peek() != EOF)
  {
    int index = readService(in, services, compressedServices);
    if(index == -1)
      break;
    else
    {
      lastI = max(index, (int)lastI);

      if(lastI % 5000 == 0)
        LOG(debug) << lastI << " services read";
    }
  }
  in.close();

  //output number of trains
  int count = 0;
  for(unsigned int i = 0; i <= lastI; ++i)
    count += services[i].size();
  LOG(info) << "total number of trains: " << count;

  /////////////////////////////////////////////////////////////////////////////
  //change bitfield indices
  LOG(info) << "updating bitfield indices";
  for(unsigned int i = 0; i <= lastI; ++i)
    for(unsigned int j = 0; j < services[i].size(); ++j)
      for(unsigned int k = 0; k < services[i][j].stations.size() - 1; ++k)
      {
        services[i][j].stations[k].trafficDayIndex =
          bsMan.getNewIndex(services[i][j].stations[k].trafficDayIndex);
      }
  for(unsigned int i = 0; i < compressedServices.size(); ++i)
    for(unsigned int j = 0; j < compressedServices[i].bitfields.size(); ++j)
      compressedServices[i].bitfields[j] =
        bsMan.getNewIndex(compressedServices[i].bitfields[j]);

  /////////////////////////////////////////////////////////////////////////////
  //split trains that consist of incompatible traffic days
  LOG(info) << "splitting trains";
  count = 0;
  for(unsigned int i = 0; i <= lastI; ++i)
  {
    vector<ServiceInfo> newServices;
    for(unsigned int j = 0; j < services[i].size(); ++j)
    {
      vector<ServiceInfo> res = services[i][j].split(bsMan);
      newServices.insert(newServices.end(), res.begin(), res.end());
    }
    services[i] = newServices;
    count += services[i].size();
    if(i % 5000 == 0 && i != 0)
      LOG(debug) << i << " services splitted";
  }
  LOG(info) << "trains: " << count;

  /////////////////////////////////////////////////////////////////////////////
  //merge trains
  LOG(debug) << "merging trains";
  int mergeCount = 0;
  count = 0;
  for(unsigned int i = 0; i <= lastI; ++i)
  {
    mergeCount += mergeServices(services[i], bsMan);
    count += services[i].size();
    if(i % 5000 == 0 && i != 0)
      LOG(info) << i << " services merged";
  }
  LOG(info) << "merges performed: " << mergeCount;
  LOG(info) << "trains: " << count;

  /////////////////////////////////////////////////////////////////////////////
  //generate routes
  LOG(info) << "generating routes ";
  map<RouteInfo, vector<TrainInfo> > routes;
  for(unsigned int i = 0; i <= lastI; ++i)
  {
    for(unsigned int j = 0; j < services[i].size(); ++j)
    {
      RouteInfo ri;
      TrainInfo ti;
      extractRouteInfo(services[i][j], ri, ti);
      ti.trainIndex = i;
      routes[ri].push_back(ti);
    }
    if(i % 5000 == 0 && i != 0)
      LOG(debug) << i << " services -> " << routes.size() << " routes";
  }

  /////////////////////////////////////////////////////////////////////////////
  //export routes
  LOG(info) << "exporting " << routes.size() << " routes";
  ofstream rout((prefix + string(ROUTES_FILE)).c_str());
  if(!rout)
    throw std::runtime_error("routes file for writing");
  int currIndex = 1;
  map<RouteInfo, vector<TrainInfo> >::iterator it, end = routes.end();
  for(it = routes.begin(); it != end; ++it)
  {
    vector<TrainInfo>& trains = it->second;
    assert(trains.size() > 0);
    assert(it->first.stations.size() > 0);
    rout << "r" << currIndex++ << " " << it->first.stations.size()
         << " " << trains.size() << "\n";
    rout << " ";
    it->first.write(rout);
    rout << "\n";
    for(unsigned int i = 0; i < trains.size(); ++i)
    {
      rout << "  t" << i << " ";
      trains[i].write(rout);
      rout << "\n";
    }
  }
  rout.close();

  /////////////////////////////////////////////////////////////////////////////
  //export bitsets
  LOG(debug) << "exporting bitsets";
  ofstream bfout;
  bfout.exceptions(std::ios_base::failbit);
  bfout.open((prefix + NEW_BITFIELD_FILE).c_str());
  bsMan.write(bfout);
  bfout.close();
}

}  // namespace td

///////////////////////////////////////////////////////////////////////////////
//                             ROUTES FORMAT                                 //
///////////////////////////////////////////////////////////////////////////////
//for each route:                                                            //
//                                                                           //
//r[route-index] [#stations] [#trains]                                       //
// l[location1] a[skipArrival] d[skipDeparture] [family], l[location2] ...   //
//  t1 a[arrival time] [arrival track] d[departure time] [departure track]   //
//    /[traffic day index] [transfer class] [linienzuordnung]                //
//    /[#attributes] [att1] ... [attN]                                       //
//    /[train nr]%[string evaNr]|[lineIdentifier], a...                      //
//  t2 ...                                                                   //
//  .                                                                        //
//  .                                                                        //
//  tN ...                                                                   //
//                                                                           //
//note:                                                                      //
// "/" denotes line-continuation (no line break)                             //
// the entries starting with "a" are omitted for the first station           //
// the entries starting with "d" are omitted for the last station            //
///////////////////////////////////////////////////////////////////////////////

