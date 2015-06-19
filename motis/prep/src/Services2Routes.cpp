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

#include "motis/core/common/logging.h"
#include "motis/loader/files.h"
#include "motis/loader/bitset_manager.h"

using namespace std;

namespace td {

using namespace logging;

struct train_station_info
{
  int a_time;
  int a_platform;
  int d_time;
  int d_platform;
  int traffic_day_index;
  int transfer_class;
  int linienzuordnung;
  vector<int> attributes;
  int train_nr;
  string eva_nr;
  string line_identifier;

  void write(ostream& out, bool first, bool last) const
  {
    if(!first)
      out << "a" << a_time << " " << a_platform;
    if(!first && !last)
      out << " ";
    if(!last)
    {
      out << "d" << d_time << " " << d_platform
          << " " << traffic_day_index
          << " " << transfer_class << " " << linienzuordnung << " "
          << attributes.size();
      for(unsigned int i = 0; i < attributes.size(); ++i)
        out << " " << attributes[i];
      out << " " << train_nr << "%" << eva_nr << "|" << line_identifier << "|";
    }
  }
};

struct train_info
{
  int train_index;
  vector<train_station_info> stations;

  void write(ostream& out) const
  {
    out << "s" << train_index << " ";
    for(unsigned int i = 0; i < stations.size(); ++i)
    {
      stations[i].write(out, i == 0, i == stations.size() - 1);
      if(i < stations.size() - 1)
        out << ", ";
    }
  }
};

struct route_station_info
{
  int location;
  bool skip_arrival;
  bool skip_departure;
  int family;

  bool operator==(const route_station_info& o) const
  { return o.location == location && o.skip_arrival == skip_arrival &&
           o.skip_departure == skip_departure && o.family == family; }

  bool operator<(const route_station_info& o) const
  {
    if(location != o.location)
      return location < o.location;
    if(skip_arrival != o.skip_arrival)
      return skip_arrival < o.skip_arrival;
    if(skip_departure != o.skip_departure)
      return skip_departure < o.skip_departure;
    if(family != o.family)
      return family < o.family;
    else
      return false;
  }

  void write(ostream& out, bool first, bool last) const
  {
    out << "l" << location;
    if(!first)
      out << " a" << skip_arrival;
    if(!last)
      out << " d" << skip_departure << " " << family;
  }
};

struct route_info
{
  vector<route_station_info> stations;

  bool operator==(const route_info& o) const
  { return stations == o.stations; }

  bool operator<(const route_info& o) const
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

struct service_station_info
{
  int location;
  bool skip_arrival;
  int a_time;
  int a_platform;
  bool skip_departure;
  int d_time;
  int d_platform;
  int traffic_day_index;
  int transfer_class;
  int linienzuordnung;
  int family;
  vector<int> attributes;
  int train_nr;
  string eva_nr;
  string line_identifier;
};

struct service_info
{
  vector<service_station_info> stations;

  bool operator<(const service_info& o) const
  { return stations[0].d_time < o.stations[0].d_time; }

   /**
   * returns a vector that stores how many night/day shifts have ocuured up
   * to station i. needed for splitting and merging.
   */
  vector<int> get_traffic_day_shifts()
  {
    //the last station has no traffic day info
    vector<int> ret(stations.size() - 1, 0);

    int shifts = 0;
    for(unsigned int i = 1; i < stations.size() - 1; ++i)
    {
      if(stations[i].d_time < stations[i - 1].d_time)
        ++shifts;
      ret[i] = shifts;
    }

    return ret;
  }

  /**
   * splits itself into several service_infos which are consistent in their
   * traffic days.
   * @return vector with the new service_infos
   */
  vector<service_info> split(bitset_manager& bm)
  {
    //vector to return
    vector<service_info> vsi;

    //get night/day shifts so that we can shift all traffic_day_indices before
    //comparing them
    vector<int> shifts = get_traffic_day_shifts();

    //check for each pair of adjacent stations whether their traffic days differ
    for(unsigned int i = 0; i < stations.size() - 2; ++i)
    {
      //compare the two shifted masks ignoring the last n bits where
      //n is the difference in shifts
      //after shifting we are unsure about the bits which were shifted in
      //so we cannot compare them
      if(!bm.equals(bm.shift_l(stations[i].traffic_day_index, shifts[i]),
                    bm.shift_l(stations[i + 1].traffic_day_index, shifts[i + 1]),
                    shifts[i + 1] - shifts[i]))
      {
        int common_traffic_days =
          bm.common_bitset(bm.shift_l(stations[i].traffic_day_index, shifts[i]),
                     bm.shift_l(stations[i + 1].traffic_day_index, shifts[i + 1]));
        //si2 will contain the incompatible traffic days of the first part
        service_info si2;
        for(unsigned int j = 0; j <= i + 1; ++j)
        {
          si2.stations.push_back(stations[j]);
          if(j < i + 1)
          {
            si2.stations[j].traffic_day_index =
              bm.without(si2.stations[j].traffic_day_index,
                bm.shift_r(common_traffic_days, shifts[j]));
          }
        }
        //si3 will contain the incompatible traffic days of the second part
        service_info si3;
        for(unsigned int j = i + 1; j < stations.size(); ++j)
        {
          si3.stations.push_back(stations[j]);
          if(j < stations.size() - 1)
          {
            si3.stations[j - i - 1].traffic_day_index =
            bm.without(si3.stations[j - i - 1].traffic_day_index,
                bm.shift_r(common_traffic_days, shifts[j]));
          }
        }

        //the first part cannot have anymore conflicts - add it if non-trivial
        int traffic_day_index = si2.stations[0].traffic_day_index;
        if(traffic_day_index != EMPTY_BITSET)
          vsi.push_back(si2);
        //the second part may have further conflicts -> recursion
        vector<service_info> vsi2 = si3.split(bm);
        vsi.insert(vsi.end(), vsi2.begin(), vsi2.end());

        //remove all traffic days which are not in common
        for(unsigned int j = 0; j < stations.size() - 1; ++j)
          stations[j].traffic_day_index =
            bm.common_bitset(stations[j].traffic_day_index,
                bm.shift_r(common_traffic_days, shifts[j]));

        //the common part may also have further conflicts ->recursion
        if(stations[0].traffic_day_index != EMPTY_BITSET)
        {
          vector<service_info> vsi3 = this->split(bm);
          vsi.insert(vsi.end(), vsi3.begin(), vsi3.end());
        }

        return vsi;
      }
    }

    //no more splitting needed? -> recursion anchor
    if(stations[0].traffic_day_index != EMPTY_BITSET)
      vsi.push_back(*this);

    return vsi;
  }


  /**
   * sets the traffic days of all stations to index
   */
  void set_traffic_days(int index, bitset_manager& bm)
  {
    vector<int> shifts = get_traffic_day_shifts();
    for(unsigned int i = 0; i < stations.size() - 1; ++i)
      stations[i].traffic_day_index = bm.shift_r(index, shifts[i]);
  }

  /**
   * removes the traffic days in index from all stations
   */
  service_info remove_traffic_days(int index, bitset_manager& bm)
  {
    vector<int> shifts = get_traffic_day_shifts();
    service_info ret = *this;
    for(unsigned int i = 0; i < stations.size() - 1; ++i)
      ret.stations[i].traffic_day_index =
       bm.without(ret.stations[i].traffic_day_index, bm.shift_r(index, shifts[i]));
    return ret;
  }

  /**
   * merges the two service_infos and returns the number of resulting service_info
   * objects.
   * if two objects are to be connected that have different traffic
   * days, the common set of traffic days is used for the merged service_info.
   * up to two more service_infos have to be created, that contain the remaining
   * traffic days
   * @param the service_info to merge with this
   * @param if 3 is returned this parameter contains a new service
   * @return number of services after merging.
   **/
  int merge(/*in/out*/service_info& o, /*out*/service_info& o2, bitset_manager& bm)
  {
    int size = stations.size();
    assert(size > 1);
    //the two services have to be connectible
    if(stations[size - 1].location != o.stations[0].location)
      return 2;

    int shifts = 0;
    //day/night shift?
    if(stations[size - 2].d_time > o.stations[0].d_time)
      shifts = 1;

    int index1 = stations[size - 2].traffic_day_index;
    int index2 = bm.shift_l(o.stations[0].traffic_day_index, shifts);

    //ignore shifted bits
    if(bm.equals(index1, index2, shifts))
    {
      int a_time = stations[size - 1].a_time;
      bool skip_arrival = stations[size - 1].skip_arrival;
      stations.pop_back();

      stations.insert(stations.end(), o.stations.begin(), o.stations.end());
      stations[size - 1].a_time = a_time;
      stations[size - 1].skip_arrival = skip_arrival;

      return 1;
    }
    else
    {
      int common = bm.common_bitset(index1, index2);
      //is the common set empty? (disregarding shifted bits)
      if(bm.equals(common, EMPTY_BITSET, shifts))
        return 2;

      vector<int> shifts_vec = get_traffic_day_shifts();

      o2 = o.remove_traffic_days(bm.shift_r(common, shifts), bm);

      service_info o3 =
        remove_traffic_days(bm.shift_l(common, shifts_vec[size - 2]), bm);

      set_traffic_days(bm.shift_l(common, shifts_vec[size - 2]), bm);
      o.set_traffic_days(bm.shift_r(common, shifts), bm);

      int a_time = stations[size - 1].a_time;
      bool skip_arrival = stations[size - 1].skip_arrival;
      stations.pop_back();

      stations.insert(stations.end(), o.stations.begin(), o.stations.end());
      stations[size - 1].a_time = a_time;
      stations[size - 1].skip_arrival = skip_arrival;

      int ret = 1;

      //handle o2
      if(!bm.equals(o2.stations[0].traffic_day_index, EMPTY_BITSET, shifts))
      {
        o = o2;
        ret++;
      }

      //handle o3
      if(!bm.equals(o3.stations[0].traffic_day_index, EMPTY_BITSET,
                   shifts_vec[size - 2]))
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

struct compressed_service_info
{
  vector<service_station_info> stations;
  vector<int> times, bitfields;
  int index;
};

void read_station(istream& in, service_station_info& ssi, bool first_or_last)
{
  char c;
  in >> c;

  assert(c == 'l');
  in >> ssi.location;

  in >> c;
  if(!first_or_last || c == 'a')
  {
    assert(c == 'a');
    in >> ssi.skip_arrival >> ssi.a_time;
  }
  else
    assert(c == 'd');

  if(!first_or_last)
  {
    in >> c;
    assert(c == 'd');
  }

  if(c == 'd')
  {
    in >> ssi.skip_departure >> ssi.d_time >> ssi.traffic_day_index
       >> ssi.transfer_class >> ssi.linienzuordnung >> ssi.family;
    unsigned int n;
    in >> n;
    ssi.attributes.resize(n);
    for(unsigned int i = 0; i < n; ++i)
      in >> ssi.attributes[i];
    in >> ssi.train_nr;
    in >> c;
    assert(c == '%');
    getline(in, ssi.eva_nr, '|');

    string train_name;
    getline(in, train_name, '|');

    getline(in, ssi.line_identifier, '|');

    in >> ssi.d_platform >> ssi.a_platform;
  }
}

int read_service(istream& in, vector<vector<service_info> >& services,
                vector<compressed_service_info>& compressed_services)
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
    int ntimes, n_bitfields;
    in >> ntimes;

    compressed_service_info csi;
    csi.index = index;

    csi.times.resize(ntimes);
    for(int i = 0; i < ntimes; ++i)
      in >> csi.times[i];

    in >> c;
    assert(c == 'b');
    in >> n_bitfields;
    assert(n_bitfields == ntimes);
    csi.bitfields.resize(n_bitfields);
    for(int i = 0; i < n_bitfields; ++i)
      in >> csi.bitfields[i];

    csi.stations.resize(n);
    for(unsigned int i = 0; i < n; ++i)
      read_station(in, csi.stations[i], i == 0 || i == n - 1);

    compressed_services.push_back(csi);

    return -2;
  }
  else
  {
    in.putback(c);

    service_info si;
    si.stations.resize(n);
    for(unsigned int i = 0; i < n; ++i)
      read_station(in, si.stations[i], i == 0 || i == n - 1);

    while(index >= (int)services.size())
      services.push_back(vector<service_info>());
    services[index].push_back(si);

    return index;
  }
}

int merge_services(vector<service_info>& vsi, bitset_manager& bm)
{
  int merge_count = 0;

  for(unsigned int i = 0; i < vsi.size(); ++i)
    for(unsigned int j = 0; j < vsi.size(); ++j)
    {
      if(j == i)
        continue;
      int num;
      service_info temp;
      if((num = vsi[i].merge(vsi[j], temp, bm)) != 2)
      {
        ++merge_count;
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

  return merge_count;
}

void extract_route_info(const service_info& si, route_info& ri, train_info& ti)
{
  for(unsigned int i = 0; i < si.stations.size(); ++i)
  {
    route_station_info rsi;
    rsi.location = si.stations[i].location;
    rsi.skip_arrival = si.stations[i].skip_arrival;
    rsi.skip_departure = si.stations[i].skip_departure;
    rsi.family = si.stations[i].family;
    ri.stations.push_back(rsi);

    train_station_info tsi;
    tsi.a_time = si.stations[i].a_time;
    tsi.a_platform = (i != 0) ? si.stations[i - 1].a_platform : 0;
    tsi.d_time = si.stations[i].d_time;
    tsi.d_platform = si.stations[i].d_platform;
    tsi.traffic_day_index = si.stations[i].traffic_day_index;
    tsi.transfer_class = si.stations[i].transfer_class;
    tsi.linienzuordnung = si.stations[i].linienzuordnung;
    tsi.attributes = si.stations[i].attributes;
    tsi.train_nr = si.stations[i].train_nr;
    tsi.eva_nr = si.stations[i].eva_nr;
    tsi.line_identifier = si.stations[i].line_identifier;
    ti.stations.push_back(tsi);
  }
}

const char* BITFIELD_FILE = ".bitfields.txt";
const char* NEW_BITFIELD_FILE = ".route.bitfields.txt";

void services2routes(std::string const& prefix)
{
  /////////////////////////////////////////////////////////////////////////////
  //read bitfields
  LOG(info) << "reading bitfields";
  ifstream bin;
  bin.exceptions(std::ios_base::failbit);
  bin.open((prefix + BITFIELD_FILE).c_str());
  bitset_manager bs_man(bin);
  bin.close();
  LOG(info) << "done";

  ifstream in;
  in.open((prefix + SERVICES_FILE).c_str());

  // the events of a train in the services file does not necessarily
  // appear in consecutive lines and can be separated.
  // therefore, separated parts of the same train have to be merged
  // to one complete train.
  // the vector 'services' contains all trains in the services file.
  // each element is a vector containing all parts of the same train.
  vector<vector<service_info>> services(100000);
  vector<compressed_service_info> compressed_services;

  /////////////////////////////////////////////////////////////////////////////
  //read services
  LOG(debug) << "reading services";
  unsigned int last_i = 0;
  while(!in.eof() && in.peek() != EOF)
  {
    int index = read_service(in, services, compressed_services);
    if(index == -1)
      break;
    else
    {
      last_i = max(index, (int)last_i);

      if(last_i % 5000 == 0)
        LOG(debug) << last_i << " services read";
    }
  }
  in.close();

  //output number of trains
  int count = 0;
  for(unsigned int i = 0; i <= last_i; ++i)
    count += services[i].size();
  LOG(info) << "total number of trains: " << count;

  /////////////////////////////////////////////////////////////////////////////
  //change bitfield indices
  LOG(info) << "updating bitfield indices";
  for(unsigned int i = 0; i <= last_i; ++i)
    for(unsigned int j = 0; j < services[i].size(); ++j)
      for(unsigned int k = 0; k < services[i][j].stations.size() - 1; ++k)
      {
        services[i][j].stations[k].traffic_day_index =
          bs_man.get_new_index(services[i][j].stations[k].traffic_day_index);
      }
  for(unsigned int i = 0; i < compressed_services.size(); ++i)
    for(unsigned int j = 0; j < compressed_services[i].bitfields.size(); ++j)
      compressed_services[i].bitfields[j] =
        bs_man.get_new_index(compressed_services[i].bitfields[j]);

  /////////////////////////////////////////////////////////////////////////////
  //split trains that consist of incompatible traffic days
  LOG(info) << "splitting trains";
  count = 0;
  for(unsigned int i = 0; i <= last_i; ++i)
  {
    vector<service_info> new_services;
    for(unsigned int j = 0; j < services[i].size(); ++j)
    {
      vector<service_info> res = services[i][j].split(bs_man);
      new_services.insert(new_services.end(), res.begin(), res.end());
    }
    services[i] = new_services;
    count += services[i].size();
    if(i % 5000 == 0 && i != 0)
      LOG(debug) << i << " services splitted";
  }
  LOG(info) << "trains: " << count;

  /////////////////////////////////////////////////////////////////////////////
  //merge trains
  LOG(debug) << "merging trains";
  int merge_count = 0;
  count = 0;
  for(unsigned int i = 0; i <= last_i; ++i)
  {
    merge_count += merge_services(services[i], bs_man);
    count += services[i].size();
    if(i % 5000 == 0 && i != 0)
      LOG(info) << i << " services merged";
  }
  LOG(info) << "merges performed: " << merge_count;
  LOG(info) << "trains: " << count;

  /////////////////////////////////////////////////////////////////////////////
  //generate routes
  LOG(info) << "generating routes ";
  map<route_info, vector<train_info> > routes;
  for(unsigned int i = 0; i <= last_i; ++i)
  {
    for(unsigned int j = 0; j < services[i].size(); ++j)
    {
      route_info ri;
      train_info ti;
      extract_route_info(services[i][j], ri, ti);
      ti.train_index = i;
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
  int curr_index = 0;
  map<route_info, vector<train_info> >::iterator it, end = routes.end();
  for(it = routes.begin(); it != end; ++it)
  {
    vector<train_info>& trains = it->second;
    assert(trains.size() > 0);
    assert(it->first.stations.size() > 0);
    rout << "r" << curr_index++ << " " << it->first.stations.size()
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
  bs_man.write(bfout);
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

