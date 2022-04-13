#ifndef ABSSTATE_H
#define ABSSTATE_H

#include <cassert>
#include <cstddef>
#include <cstdlib>
#include <iostream>
#include <list>
#include <map>
#include <sstream>
#include <string>

#include "Address.h"
// Forward declarations

namespace cacheAnaPass {
class AbstractState;

} // namespace cacheAnaPass

class AbstractState {
public: // everything is public, because IDGAF
  /**
   * @brief Containing all Abstract Cache Tags.
   *        Key of the list has no Meaning.
   *
   */
  struct Entry {
    std::list<unsigned int> Blocks;
  };

  /**
   * @brief Cache Set, Key is the Age of the Entries.
   *
   */
  struct Set {
    // uInt in this map is the Age.
    std::map<unsigned int, Entry> Entries;
  };

  /**
   * @brief Cache Sets, key is the Set number [0-15], derived from Address.
   *
   */
  std::map<unsigned int, Set> Sets;

  AbstractState() {}
  AbstractState(Address Addr) { Sets[Addr.Index].Entries[0] = {{Addr.Tag}}; }

  /**
   * @brief Updates the AbstractState with given Address
   * 
   * @param Addr , Address
   */
  void update(Address Addr) {
    for (int i = 3; i > 0; i--) {
      Sets[Addr.Index].Entries[i] = Sets[Addr.Index].Entries[i - 1];
    }
    Sets[Addr.Index].Entries[0] = {{Addr.Tag}};
  }

  /**
   * @brief Executes an Must LRU Join on the AbstractCacheState
   * 
   * @param In, AbstractState that gets joined into the State.
   */
  void mustJoin(AbstractState In) {
    // I can not assume every element exists(not using static DataTypes).

    // 2. join all Sets from In, that don't exist here into current State.
    for (auto Set2 : In.Sets) {
      unsigned int Index = Set2.first;
      if (Sets.find(Index) == Sets.end()) {
        Sets[Index] = Set2.second;
      }
    }
    // 2. Iterator can now be used without missing any Set in In.
    // this could be way more efficient but again IDGAF
    for (auto Set1 : Sets) {
      unsigned int Index = Set1.first;
      // See if corresponding Set2 exists
      if (In.Sets.find(Index) == In.Sets.end()) {
        break;
      }
      // AbstractState::Set
      auto Set2 = In.Sets[Index];
      // Check every Entry in Set[Index].
      for (auto E1 : Set1.second.Entries) {
        unsigned int Age1 = E1.first;
        for (auto E2 : Set2.Entries) {
          unsigned int Age2 = E2.first;
          if (E1.second.Blocks.empty())
            break;
          for (unsigned int Block1 : E1.second.Blocks) {
            int NewAge = -1;
            // Find Block1 in In.
            if (E2.second.Blocks.empty())
              break;
            for (unsigned int Block2 : E2.second.Blocks) {
              if (E1.second.Blocks == E2.second.Blocks)
                break;
              if (Block1 == Block2)
                NewAge = (Age1 > Age2) ? Age1 : Age2;
            }
            // If current Entry is already older, do nothing
            if (NewAge != Age1) {
              // Other wise move or remove Block1
              if (NewAge >= 0) {
                Sets[Index].Entries[NewAge].Blocks.push_back(Block1);
                E1.second.Blocks.remove(Block1);
              } else {
                E1.second.Blocks.remove(Block1);
              }
              if (E1.second.Blocks.empty())
                break;
            }
          }
        }
      }
    }
  }

};     // namespace
#endif // STATE_H