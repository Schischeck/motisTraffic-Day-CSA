#include "motis/loader/bitset_manager.h"

#include <cassert>

using namespace std;
using namespace td;

bitset_manager::bitset_manager(istream& in)
{
  //add empty bitset
  schedule_bitset b;
  add(b);
  //b.flip();
  //add(b);
  _new_indices[1] = 0;

  while(!in.eof() && in.peek() != EOF)
  {
    int i;
    char c;
    in >> i >> c;
    if(in.eof())
      break;
    string s;
    getline(in, s);
    s = s.substr(0, s.size() - 1);
    _real_size = s.size();
    assert(_real_size <= MAXDAYS);
    schedule_bitset bs(s);
    _new_indices[i] = add(bs);
  }
}

bitset_index bitset_manager::get_new_index(bitset_index old)
{
  if(_new_indices.find(old) == _new_indices.end())
  {
    //cerr << "RETURNED -1 !!! for " << old << endl;
    //return 0;
    return bitset_index();
  }
  else
    return _new_indices[old];
}

bitset_index bitset_manager::common_bitset(bitset_index first, bitset_index second)
{
  schedule_bitset bs;
  bs |= _bitsets[first];
  bs &= _bitsets[second];
  return add(bs);
}

bitset_index bitset_manager::difference_bitset(bitset_index larger, bitset_index smaller)
{
  schedule_bitset bs;
  bs |= _bitsets[larger];
  bs &= _bitsets[smaller];
  schedule_bitset bs2;
  bs2 |= _bitsets[larger];
  bs2 ^= bs;
  return add(bs2);
}

bitset_index bitset_manager::without(bitset_index large, bitset_index small)
{
  schedule_bitset bs;
  bs |= _bitsets[small];
  bs.flip();
  bs &= _bitsets[large];
  return add(bs);
}

bitset_index bitset_manager::shift_l(bitset_index i, int amount)
{
  schedule_bitset bs;
  bs |= _bitsets[i];
  bs <<= amount;
  for(int j = 0; j < amount; ++j)
    bs[j] = bs[_real_size + j] = 0;
  return add(bs);
}

bitset_index bitset_manager::shift_r(bitset_index i, int amount)
{
  schedule_bitset bs;
  bs |= _bitsets[i];
  bs >>= amount;
  for(int j = 0; j < amount; ++j)
    bs[bs.size() - 1 - j] = bs[_real_size - j - 1] = 0;
  return add(bs);
}

schedule_bitset bitset_manager::get_bitset(bitset_index i)
{ return _bitsets[i]; }

bool bitset_manager::equals(bitset_index a, bitset_index b, int ignoring)
{
  string s1 = _bitsets[a].to_string();
  s1 = s1.substr(s1.size() - _real_size, _real_size - ignoring);
  string s2 = _bitsets[b].to_string();
  s2 = s2.substr(s2.size() - _real_size, _real_size - ignoring);
  return s1 == s2;
}

int bitset_manager::get_next_possible_day(bitset_index bi, int day) const
{
  while(day <= _real_size && _bitsets[bi][_real_size - day] == 0)
    ++day;
  if(day > _real_size)
    return -1;
  else
    return day;
}

void bitset_manager::write(bitset_index bi, std::ostream& out) const
{
  out << _bitsets[bi].to_string().substr(MAXDAYS - _real_size, _real_size + 1);
}

void bitset_manager::write(ostream& out) const
{
  for(unsigned int i = 0; i < _bitsets.size(); ++i)
  {
    string s = _bitsets[i].to_string();
    s = s.substr(s.size() - _real_size, _real_size);
    out << i << "%" << s << "|";
    if(i < _bitsets.size() - 1)
      out << endl;
  }
}

std::vector<int> bitset_manager::get_active_day_indices(bitset_index bi) const
{
  std::vector<int> ret;
  for (int day_index = 0; day_index < _real_size; ++day_index)
    if (has_day_i(bi, day_index))
      ret.emplace_back(day_index);
  return ret;
}

int bitset_manager::add(schedule_bitset b)
{
  if(_indices.find(b.to_string()) == _indices.end())
  {
    int new_i = _bitsets.size();
    _bitsets.push_back(b);
    _indices[b.to_string()] = new_i;
    return new_i;
  }
  else
    return _indices[b.to_string()];
}


