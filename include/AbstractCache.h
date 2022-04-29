
#ifndef ABSTRACHTCACHESTATE_H
#define ABSTRACHTCACHESTATE_H

#include <cassert>
#include <cstddef>
#include <fstream>
#include <iostream>
#include <list>
#include <map>
#include <ostream>
#include <utility>

#include <llvm/IR/BasicBlock.h>
#include <llvm/Support/raw_ostream.h>

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
  unsigned int NumberOfNodes = 0;

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

  unsigned int addEmptyNode(unsigned int NodeAddr) {
    Nodes[NumberOfNodes++] = AbstractState(NodeAddr);
    return NumberOfNodes;
  }

  /**
   * @brief Unroll Loops.
   *
   * @param NodeNr
   */
  void unrollLoops() {
    for (auto NodePair : Nodes) {
      unsigned int NodeNr = NodePair.first;
      if (NodeNr == 34) {
        llvm::outs() << "HI\n";
      }
      bool IsLoopHead = false;
      bool FoundLoopBody = false;
      bool Verbose = true;
      std::list<unsigned int> LoopBody;
      if (Nodes[NodeNr].Predecessors.size() > 1) {
        IsLoopHead = true;
        // is loop head?
        for (unsigned int Pre : Nodes[NodeNr].Predecessors) {
          if (Pre > NodeNr) {
            // Might be loop head.
            // check if all States between Pre and NodeNr are a coherent set.
            for (uint I = NodeNr; I < Pre; I++) {
              LoopBody.push_back(I);
              for (uint Succ : Nodes[I].Successors) {
                if (Succ > Pre) {
                  //  Set is not coherent
                  IsLoopHead = false;
                  break;
                }
              }
              FoundLoopBody = true;
            }
            LoopBody.push_back(Pre);
          } else if (!FoundLoopBody) {
            // If no coherent Loopbody exist we cannot unroll.
            LoopBody.clear();
            IsLoopHead = false;
          }
        }
      }
      if (IsLoopHead && Verbose) {
        llvm::outs() << "Found LoopHead @: " << NodeNr << "\n";
        llvm::outs() << "With Body: {\n";
        int I = 1;
        for (auto Node : LoopBody) {
          llvm::outs() << Node << ", ";
          if (!(I++ % 5)) {
            llvm::outs() << "\n";
          }
        }
        llvm::outs() << "}\n";
      }
      // Found Loop Head and Body!
      // TODO: Now unroll
      // Add empty unrolled Nodes
      // Map points from OrigNode To Unrolled Node.
      std::map<unsigned int, unsigned int> OrigNodeToUnrolledNode;
      for (auto Node : LoopBody) {
        // Node to unroll
        AbstractState UnrolledNode(Nodes[Node]);
        UnrolledNode.setUnrolled(1);
        Nodes[NumberOfNodes++] = UnrolledNode;
        OrigNodeToUnrolledNode[Node] = NumberOfNodes;
      }

      unsigned int LoopHead = LoopBody.front();
      LoopBody.pop_front();
      unsigned int LoopTail = LoopBody.back();
      LoopBody.pop_back();
      for (auto Node : LoopBody) {
        for (auto Succ : Nodes[Node].Successors) {
          // Add All successors to unrolled Node
          Nodes[OrigNodeToUnrolledNode[Node]].Successors.push_back(
              OrigNodeToUnrolledNode[Succ]);
        }
        for (auto Pre : Nodes[Node].Predecessors) {
          // Add All predecessors to unrolled Node
          Nodes[OrigNodeToUnrolledNode[Node]].Successors.push_back(
              OrigNodeToUnrolledNode[Pre]);
        }
      }
    }
    return;
  }

  /**
   * @brief Idea fill the graph with the node and perform loop unrolling.
   *
   * @param NodeNr
   */
  void fillAbstractCache(unsigned int NodeNr) {
    // if(isLoopHead(NodeNr))
    // unrollLoop(NodeNr);
    Nodes[NodeNr].Computed = true;
    for (unsigned int SuccNr : Nodes[NodeNr].Successors) {
      Nodes[SuccNr];
      if (Nodes[SuccNr].Computed) {
        // Join don't call
        // TODO fix Join
        Nodes[SuccNr].mustJoin(Nodes[NodeNr]); // maybe fill
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
      auto Predecessor = Nodes[E.first];
      for (unsigned int SuccessorAddr : E.second) {
        // When successors Address is in predecessor, we have a Hit.
        Hits += Predecessor.isHit(Address(SuccessorAddr)) ? 1 : 0;
      }
    }
    return Hits;
  }

  unsigned int collectMisses() {
    unsigned int Misses = 0;
    for (auto const &E : Edges) {
      auto Predecessor = Nodes[E.first];
      for (unsigned int SuccessorAddr : E.second) {
        // When successors Address is in predecessor, we have a Hit.
        Misses += Predecessor.isHit(Address(SuccessorAddr)) ? 0 : 1;
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
    bool PrintOld = true;
    std::ofstream DotFile;
    DotFile.open("out.dot");
    DotFile << "digraph g {"
            << "\n";
    for (auto const &E : Edges) {
      for (unsigned int To : E.second) {
        if (PrintOld) {
          DotFile << E.first << " -> " << To << "\n";
        } else {
          DotFile << Nodes[E.first].Addr << "." << Nodes[E.first].Unrolled
                  << " -> " << Nodes[To].Addr << "." << Nodes[To].Unrolled
                  << "\n";
        }
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