///////////////////////////////////////////////////////////////////////////////
// task: manage bitsets indices and create new needed bitsets on the fly     //
//                                                                           //
// author: yann disser (skratchie@gmx.de)                                    //
//                                                                           //
// creation date: 1.8.2007                                                   //
///////////////////////////////////////////////////////////////////////////////

#ifndef TD_BITSET_MANAGER_H
#define TD_BITSET_MANAGER_H TD_BITSET_MANAGER_H

#include <bitset>
#include <vector>
#include <map>
#include <ostream>
#include <istream>

namespace td {

const int MAXDAYS = 64;
typedef std::bitset<MAXDAYS> schedule_bitset;
typedef int bitset_index;

const int EMPTY_BITSET = 0;

class bitset_manager {
public:
  bitset_manager() = default;

  /**
   * reads the bitset data from the given stream.
   * @param the input stream from which to read.
   */
  bitset_manager(std::istream& in);

  /**
   * @param the old bitset index
   * @return the new index for the bitset formerly indexed by old
   */
  bitset_index get_new_index(bitset_index old);

  /**
   * computes and stores the AND of the two given bitsets.
   * @return the result of the AND operation on the two bitsets
   */
  bitset_index common_bitset(bitset_index first, bitset_index second);

  /**
   * computes a bitset in which the bits are true, that are true in larger
   * and false in smaller.
   * @return the resulting bitset index
   */
  bitset_index difference_bitset(bitset_index larger, bitset_index smaller);

  /**
   * computes a bitset in which all the bits of the second set are set to zero
   * @return the resulting bitset index
   */
  bitset_index without(bitset_index large, bitset_index small);

  /**
   * computes a left-shifted bitset filling with 0
   * @return the resulting bitset index
   */
  bitset_index shift_l(bitset_index i, int amount);

  /**
   * computes a right-shifted bitset filling with 0
   * @return the resulting bitset index
   */
  bitset_index shift_r(bitset_index i, int amount);

  /**
   * returns bitset i.
   */
  schedule_bitset get_bitset(bitset_index i);

  /**
   * returns the indices of all active days in the bitfield.
   */
  std::vector<int> get_active_day_indices(bitset_index i) const;

  /**
   * compares two bitstrings.
   * @param bitstring1
   * @param bitstring2
   * @param number of trailing bits which are to be ignored
   * @return whether the two bitstrings are equal everywhere except at the
   *         ignored bits
   */
  bool equals(bitset_index a, bitset_index b, int ignoring);

  /**
   * returns whether the bitset is '1' for the given day.
   * note: the last day of the work-period is on the very right!
   */
  inline bool has_day_i(bitset_index bi, int i) const {
    if (i > _real_size) return false;
    return _bitsets[bi][_real_size - 1 - i] == 1;
  }

  /**
   * returns the index of the next day for which the bitset has a '1'.
   * @return -1 if there is no such day.
   */
  int get_next_possible_day(bitset_index bi, int day) const;

  /**
  * exports a single bitfield to the given stream.
  */
  void write(bitset_index bi, std::ostream& out) const;

  /**
   * exports the bitfields to the given stream.
   */
  void write(std::ostream& out) const;

private:
  // adds the bitset b and returns its index
  int add(schedule_bitset b);

  // maps the bitsets to their indices
  std::map<std::string, int> _indices;

  // contains all bitsets
  std::vector<schedule_bitset> _bitsets;

  // maps old indices to new ones
  std::map<int, int> _new_indices;

  // the length of the represented bitsets
  int _real_size;
};
}

#endif  // TD_BITSET_MANAGER_H
