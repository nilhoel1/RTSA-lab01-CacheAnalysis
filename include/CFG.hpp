#include <list>
#include <llvm/IR/Value.h>
#include <map>

#include "llvm/Support/raw_ostream.h"

class CFG {

  struct vertice {
    std::list<llvm::Value> Ins;
    std::list<llvm::Value> Outs;
  };

  std::map<llvm::StringRef, vertice> Vertices;
};