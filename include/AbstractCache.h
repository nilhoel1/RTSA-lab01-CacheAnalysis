
#ifndef ABSTRACHTCACHESTATE_H
#define ABSTRACHTCACHESTATE_H

#include <cassert>
#include <cstddef>
#include <llvm/Support/raw_ostream.h>
#include <map>
#include <ostream>
#include <utility>

#include "AbstractState.h"
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

  void addEdge(unsigned int From, unsigned int To) {
    Edges[From].push_back(To);
  }

  void addEmptyNode(unsigned int NodeAddr) {
    Nodes[NodeAddr] = AbstractState();
  }

  void addNode(unsigned int NodeAddr) {
    Nodes[NodeAddr] = AbstractState(Address(NodeAddr));
  }

  void fillAbstractCache() {
    for (auto E : Edges) {
      unsigned int FromNodeNr = E.first;
      AbstractState FromNode = Nodes[FromNodeNr];
      for (unsigned int ToNodeNr : E.second) {
        AbstractState ToNode = Nodes[ToNodeNr];
        // TODO Join FromNode into ToNode
        ToNode.mustJoin(FromNode);
        ToNode.update(Address(FromNodeNr));
      }
    }
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

};     // namespace
#endif // ABSTRACHTCACHESTATE_H