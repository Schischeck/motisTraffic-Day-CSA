#include "BitsetManager.h"

#include <cassert>

using namespace std;
using namespace td;

BitsetManager::BitsetManager(istream& in)
{
  //add empty bitset
  Bitset b;
  add(b);
  //b.flip();
  //add(b);
  _newIndices[1] = 0;

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
    _realSize = s.size();
    assert(_realSize <= MAXDAYS);
    Bitset bs(s);
    _newIndices[i] = add(bs);
  }
}

BitsetIndex BitsetManager::getNewIndex(BitsetIndex old)
{
  if(_newIndices.find(old) == _newIndices.end())
  {
    //cerr << "RETURNED -1 !!! for " << old << endl;
    //return 0;
    return BitsetIndex();
  }
  else
    return _newIndices[old];
}

BitsetIndex BitsetManager::commonBitset(BitsetIndex first, BitsetIndex second)
{
  Bitset bs;
  bs |= _bitsets[first];
  bs &= _bitsets[second];
  return add(bs);
}

BitsetIndex BitsetManager::differenceBitset(BitsetIndex larger, BitsetIndex smaller)
{
  Bitset bs;
  bs |= _bitsets[larger];
  bs &= _bitsets[smaller];
  Bitset bs2;
  bs2 |= _bitsets[larger];
  bs2 ^= bs;
  return add(bs2);
}

BitsetIndex BitsetManager::without(BitsetIndex large, BitsetIndex small)
{
  Bitset bs;
  bs |= _bitsets[small];
  bs.flip();
  bs &= _bitsets[large];
  return add(bs);
}

BitsetIndex BitsetManager::shiftL(BitsetIndex i, int amount)
{
  Bitset bs;
  bs |= _bitsets[i];
  bs <<= amount;
  for(int j = 0; j < amount; ++j)
    bs[j] = bs[_realSize + j] = 0;
  return add(bs);
}

BitsetIndex BitsetManager::shiftR(BitsetIndex i, int amount)
{
  Bitset bs;
  bs |= _bitsets[i];
  bs >>= amount;
  for(int j = 0; j < amount; ++j)
    bs[bs.size() - 1 - j] = bs[_realSize - j - 1] = 0;
  return add(bs);
}

Bitset BitsetManager::getBitset(BitsetIndex i)
{ return _bitsets[i]; }

bool BitsetManager::equals(BitsetIndex a, BitsetIndex b, int ignoring)
{
  string s1 = _bitsets[a].to_string();
  s1 = s1.substr(s1.size() - _realSize, _realSize - ignoring);
  string s2 = _bitsets[b].to_string();
  s2 = s2.substr(s2.size() - _realSize, _realSize - ignoring);
  return s1 == s2;
}

int BitsetManager::getNextPossibleDay(BitsetIndex bi, int day) const
{
  while(day <= _realSize && _bitsets[bi][_realSize - day] == 0)
    ++day;
  if(day > _realSize)
    return -1;
  else
    return day;
}

void BitsetManager::write(BitsetIndex bi, std::ostream& out) const
{
  out << _bitsets[bi].to_string().substr(MAXDAYS - _realSize, _realSize + 1);
}

void BitsetManager::write(ostream& out) const
{
  for(unsigned int i = 0; i < _bitsets.size(); ++i)
  {
    string s = _bitsets[i].to_string();
    s = s.substr(s.size() - _realSize, _realSize);
    out << i << "%" << s << "|";
    if(i < _bitsets.size() - 1)
      out << endl;
  }
}

std::vector<int> BitsetManager::getActiveDayIndices(BitsetIndex bi) const
{
  std::vector<int> ret;
  for (int dayIndex = 0; dayIndex < _realSize; ++dayIndex)
    if (hasDayI(bi, dayIndex))
      ret.emplace_back(dayIndex);
  return ret;
}

int BitsetManager::add(Bitset b)
{
  if(_indices.find(b.to_string()) == _indices.end())
  {
    int newI = _bitsets.size();
    _bitsets.push_back(b);
    _indices[b.to_string()] = newI;
    return newI;
  }
  else
    return _indices[b.to_string()];
}


