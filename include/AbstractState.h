#ifndef ABSSTATE_H
#define ABSSTATE_H

#include <cassert>
#include <cstddef>
#include <cstdlib>
#include <iostream>
#include <list>
#include <llvm/Support/raw_ostream.h>
#include <map>
#include <ostream>
#include <sstream>
#include <string>

#include "Address.h"
// Forward declarations

namespace cacheAnaPass {
class AbstractState;

} // namespace cacheAnaPass

class AbstractState {
public: // everything is public, because IDGAF
  std::list<unsigned int> Successors;
  std::list<unsigned int> Predecessors;

  unsigned int Addr;

  bool computed = false;

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
    std::map<unsigned int, Entry> Associativity;
  };

  /**
   * @brief Cache Sets, key is the Set number [0-15], derived from Address.
   *
   */
  std::map<unsigned int, Set> Sets;

  AbstractState(AbstractState const &Copy) {
    for (auto S : Copy.Sets) {
      unsigned int SetNr = S.first;
      for (auto E : S.second.Associativity) {
        unsigned int Age = E.first;
        for (auto B : E.second.Blocks) {
          Sets[SetNr].Associativity[Age].Blocks.push_back(B);
        }
      }
    }
  }

  AbstractState() {}

  AbstractState(unsigned int AddressIn) { Addr = AddressIn; }

  AbstractState(Address Addr) { Sets[Addr.Index].Associativity[0] = {{Addr.Tag}}; }

  /**
   * @brief Checks if Address Addr is in Cache
   *
   * @param Addr Address to check.
   * @return true CacheState contains Address Addr
   * @return false CacheState does not contain Address Addr
   */
  bool isHit(Address Addr) {
    for (auto E : Sets[Addr.Index].Associativity) {
      for (auto B : E.second.Blocks) {
        if (B == Addr.Tag)
          return true;
      }
    }
    return false;
  }

  /**
   * @brief Updates the AbstractState with given Address
   *
   * @param Addr , Address
   */
  void update(Address Addr) {
    for (int i = 3; i > 0; i--) {
      Sets[Addr.Index].Associativity[i] = Sets[Addr.Index].Associativity[i - 1];
    }
    Sets[Addr.Index].Associativity[0].Blocks = {Addr.Tag};
  }

  /**
   * @brief Updates the AbstractState with given AbstractState
   *
   * @param UpdateState, State that gets merged into State with Age+1.
   */
  void update(AbstractState UpdateState) {
    for (auto S : UpdateState.Sets) {
      unsigned int Index = S.first;
      for (auto E : S.second.Associativity) {
        unsigned int Age = E.first + 1;
        // If updated age is greater 4 The Tag is no longer in Cache.
        // Due to associativity of 4 per set.
        if (Age >= 4)
          break;
        for (auto B : E.second.Blocks) {
          Sets[Index].Associativity[Age].Blocks.push_back(B);
        }
      }
    }
  }

  /**
   * @brief Fills the AbstractState PreState and PreAddress.
   *
   * @param PreState, State that fills this state.
   *
   * @param PreAddr Address of PreState
   */
  void fill(AbstractState PreState, Address PreAddr) {
    for (auto S : PreState.Sets) {
      unsigned int Index = S.first;
      for (auto E : S.second.Associativity) {
        unsigned int Age = E.first + 1;
        // If updated age is greater 4 The Tag is no longer in Cache.
        // Due to associativity of 4 per set.
        if (Age >= 4)
          break;
        for (auto B : E.second.Blocks) {
          Sets[Index].Associativity[Age].Blocks.push_back(B);
        }
      }
    }
    Sets[PreAddr.Index].Associativity[0].Blocks.push_back(PreAddr.Tag);
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
      for (auto E1 : Set1.second.Associativity) {
        unsigned int Age1 = E1.first;
        for (auto E2 : Set2.Associativity) {
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
                llvm::outs() << "NonEmtyJoin!\n";
              NewAge = (Age1 > Age2) ? Age1 : Age2;
            }
            // If current Entry is already older, do nothing
            if (NewAge != Age1) {
              // Other wise move or remove Block1
              if (NewAge >= 0) {
                Sets[Index].Associativity[NewAge].Blocks.push_back(Block1);
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

  void dump() {
    llvm::outs() << Addr << " {\n";
    llvm::outs() << "Predecessors: ";
    for (auto PreNr : Predecessors) {
      llvm::outs() << PreNr << " ";
    }
    llvm::outs() << "\n";

    llvm::outs() << "Successors: ";
    for (auto SuccNr : Successors) {
      llvm::outs() << SuccNr << " ";
    }
    llvm::outs() << "\n";

    for (auto SetPair : Sets) {
      llvm::outs() << "Set[" << SetPair.first << "]: \n";
      for (auto EntryPair : SetPair.second.Associativity) {
        llvm::outs() << "  Age[" << EntryPair.first << "]: ";
        for (auto Block : EntryPair.second.Blocks) {
          llvm::outs() << Block << " ";
        }
        llvm::outs() << "\n";
      }
    }
    llvm::outs() << "}\n";
  }

};     // namespace
#endif // STATE_H