
#ifndef ABSTRACHTCACHESTATE_H
#define ABSTRACHTCACHESTATE_H

#include <cassert>
#include <cstddef>
#include <fstream>
#include <iostream>
#include <llvm/IR/BasicBlock.h>
#include <llvm/Support/raw_ostream.h>
#include <map>
#include <ostream>
#include <utility>

#include "AbstractState.h"
#include "Address.h"
#include "ConcreteState.h"

// Forward declarations

namespace cacheAnaPass {
class AbstractCache;

} // namespace cacheAnaPass

class AbstractCache {
public: // everything is public, because IDGAF
  // map keys are instruction Addresses.
  std::map<unsigned int, std::list<unsigned int>> Edges;
  std::map<unsigned int, AbstractState> Nodes;

  AbstractCache() {}

  /**
   * @brief Add an Edge to the Abstract Cache
   *
   * @param Pre Predecessor Address
   * @param Suc Successor Address
   */
  void addEdge(unsigned int Pre, unsigned int Suc) {
    Edges[Pre].push_back(Suc);
    Nodes[Pre].Successors.push_back(Suc);
    Nodes[Suc].Predecessors.push_back(Pre);
  }

  void addEmptyNode(unsigned int NodeAddr) {
    Nodes[NodeAddr] = AbstractState(NodeAddr);
  }

  void fillAbstractCache(unsigned int NodeNr) {
    Nodes[NodeNr].computed = true;
    for (unsigned int SuccNr : Nodes[NodeNr].Successors) {
      Nodes[SuccNr];
      if (Nodes[SuccNr].computed) {
        // Join don't call
        Nodes[SuccNr].mustJoin(Nodes[NodeNr]);
        Nodes[SuccNr].mustJoin(AbstractState(NodeNr));
      } else {
        // Update and fill Succ
        Nodes[SuccNr].fill(Nodes[NodeNr], NodeNr);
        fillAbstractCache(SuccNr);
      }
    }
    return;
  }

  unsigned int collectHits() {
    unsigned int Hits = 0;
    for (auto const &E : Edges) {
      auto predecessor = Nodes[E.first];
      for (unsigned int SuccessorAddr : E.second) {
        // When successors Address is in predecessor, we have a Hit.
        Hits += predecessor.isHit(Address(SuccessorAddr)) ? 1 : 0;
      }
    }
    return Hits;
  }

  unsigned int collectMisses() {
    unsigned int Misses = 0;
    for (auto const &E : Edges) {
      auto predecessor = Nodes[E.first];
      for (unsigned int SuccessorAddr : E.second) {
        // When successors Address is in predecessor, we have a Hit.
        Misses += predecessor.isHit(Address(SuccessorAddr)) ? 0 : 1;
      }
    }
    return Misses;
  }

  void dumpEdges() {
    llvm::outs() << "Dumping Edges:\n";
    for (auto const &E : Edges) {
      llvm::outs() << E.first;
      bool FirstPrint = true;
      for (unsigned int To : E.second) {
        if (FirstPrint) {
          llvm::outs() << " -> " << To;
          FirstPrint = false;
        } else {
          llvm::outs() << ", " << To;
        }
      }
      llvm::outs() << "\n";
    }
  }

  void dumpDotFile() {
    std::ofstream DotFile;
    DotFile.open("out.dot");
    DotFile << "digraph g {"
            << "\n";
    for (auto const &E : Edges) {
      for (unsigned int To : E.second) {
        DotFile << E.first << " -> " << To << "\n";
      }
    }
    DotFile << "}\n";
    DotFile.close();
  }

  void dumpNodes() {
    for (auto const &E : Edges) {
      Nodes[E.first].dump();
    }
  }
};     // namespace
#endif // ABSTRACHTCACHESTATE_H