///////////////////////////////////////////////////////////////////////////////
// task: manage bitsets indices and create new needed bitsets on the fly     //
//                                                                           //
// author: Yann Disser (Skratchie@gmx.de)                                    //
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

namespace td
{

const int MAXDAYS = 64;
typedef std::bitset<MAXDAYS> Bitset;
typedef int BitsetIndex;

const int EMPTY_BITSET = 0;

class BitsetManager
{
  public:
    BitsetManager() = default;

    /**
     * Reads the bitset data from the given stream.
     * @param The input stream from which to read.
     */
    BitsetManager(std::istream& in);

    /**
     * @param The old bitset index
     * @return The new index for the bitset formerly indexed by old
     */
    BitsetIndex getNewIndex(BitsetIndex old);

    /**
     * Computes and stores the AND of the two given bitsets.
     * @return The result of the AND operation on the two Bitsets
     */
    BitsetIndex commonBitset(BitsetIndex first, BitsetIndex second);

    /**
     * Computes a bitset in which the bits are true, that are true in larger
     * and false in smaller.
     * @return The resulting bitset index
     */
    BitsetIndex differenceBitset(BitsetIndex larger, BitsetIndex smaller);

    /**
     * Computes a bitset in which all the bits of the second set are set to zero
     * @return The resulting bitset index
     */
    BitsetIndex without(BitsetIndex large, BitsetIndex small);

    /**
     * Computes a left-shifted bitset filling with 0
     * @return The resulting bitset index
     */
    BitsetIndex shiftL(BitsetIndex i, int amount);

    /**
     * Computes a right-shifted bitset filling with 0
     * @return The resulting bitset index
     */
    BitsetIndex shiftR(BitsetIndex i, int amount);

    /**
     * Returns Bitset i.
     */
    Bitset getBitset(BitsetIndex i);

    /**
     * Returns the indices of all active days in the bitfield.
     */
    std::vector<int> getActiveDayIndices(BitsetIndex i) const;

    /**
     * Compares two bitstrings.
     * @param Bitstring1
     * @param Bitstring2
     * @param Number of trailing bits which are to be ignored
     * @return Whether the two Bitstrings are equal everywhere except at the
     *         ignored bits
     */
    bool equals(BitsetIndex a, BitsetIndex b, int ignoring);

    /**
     * Returns whether the Bitset is '1' for the given day.
     * Note: the last day of the work-period is on the very right!
     */
    inline bool hasDayI(BitsetIndex bi, int i) const
    {
      if (i > _realSize)
        return false;
      return _bitsets[bi][_realSize - 1 - i] == 1;
    }

    /**
     * Returns the index of the next Day for which the Bitset has a '1'.
     * @return -1 if there is no such day.
     */
    int getNextPossibleDay(BitsetIndex bi, int day) const;

    /**
    * Exports a single Bitfield to the given stream.
    */
    void write(BitsetIndex bi, std::ostream& out) const;

    /**
     * Exports the Bitfields to the given stream.
     */
    void write(std::ostream& out) const;

  private:
    //adds the bitset b and returns its index
    int add(Bitset b);

    //maps the bitsets to their indices
    std::map<std::string, int> _indices;

    //contains all bitsets
    std::vector<Bitset> _bitsets;

    //maps old indices to new ones
    std::map<int, int> _newIndices;

    //the length of the represented bitsets
    int _realSize;
};

}

#endif //TD_BITSET_MANAGER_H

