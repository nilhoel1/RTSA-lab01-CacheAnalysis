//=============================================================================
// FILE:
//    CacheAnalysisPass.cpp
//
// DESCRIPTION:
//    Visits all functions in a module, prints their names and the number of
//    arguments via stderr. Strictly speaking, this is an analysis pass (i.e.
//    the functions are not modified). However, in order to keep things simple
//    there's no 'print' method here (every analysis pass should implement it).
//
// USAGE:
//    New PM:
//      opt -load-pass-plugin=libCacheAnalysisPass.dylib -passes=lru-misses `\`
//        -disable-output <input-llvm-file>
//
//
// License: MIT
//=============================================================================
#include "llvm/ADT/StringRef.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/CFG.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/PassManager.h"
#include "llvm/IR/Value.h"
#include "llvm/Pass.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"

#include <cstddef>
#include <cstdlib>
#include <llvm/Support/raw_ostream.h>
#include <utility>

#include "../include/AbstractCache.h"
#include "../include/CacheType.h"

using namespace llvm;

//-----------------------------------------------------------------------------
// CacheAnalysisPass implementation
//-----------------------------------------------------------------------------
// No need to expose the internals of the pass to the outside world - keep
// everything in an anonymous namespace.
namespace {

std::string typeToName(Type::TypeID Id) {
  switch (Id) {
  case Type::TypeID::ArrayTyID:
    return "ArrayTy";
  case Type::TypeID::BFloatTyID:
    return "BFloatTy";
  case Type::TypeID::FloatTyID:
    return "FloatTy";
  case Type::TypeID::DoubleTyID:
    return "DoubleTy";
  case Type::TypeID::FixedVectorTyID:
    return "FixedVectorTy";
  case Type::TypeID::FP128TyID:
    return "FP128Ty";
  case Type::TypeID::FunctionTyID:
    return "FunctionTy";
  case Type::TypeID::HalfTyID:
    return "HalfTy";
  case Type::TypeID::IntegerTyID:
    return "IntegerTy";
  case Type::TypeID::LabelTyID:
    return "LabelTy";
  case Type::TypeID::MetadataTyID:
    return "MetadataTy";
  case Type::TypeID::PointerTyID:
    return "PointerTy";
  case Type::TypeID::PPC_FP128TyID:
    return "PPC_FP128Ty";
  case Type::TypeID::ScalableVectorTyID:
    return "ScalableVectorTy";
  case Type::TypeID::StructTyID:
    return "StructTy";
  case Type::TypeID::TokenTyID:
    return "TokenTy";
  case Type::TypeID::VoidTyID:
    return "VoidTy";
  case Type::TypeID::X86_AMXTyID:
    return "X86_AMXTy";
  case Type::TypeID::X86_FP80TyID:
    return "X86_FP80Ty";
  case Type::TypeID::X86_MMXTyID:
    return "X86_MMXTy";
  }
  // should not reach here
  return nullptr;
}
// New PM implementation

// TODO2: Apply loopbounds to CFG, either from ScEv or default to 100.
// TODO3: Find longest Path, LPsolve?
// TODO4: Sum up Cache misses over longest path.
struct CacheAnalysisPass : PassInfoMixin<CacheAnalysisPass> {

  // Development Options
  bool printAddresses = false;
  bool printEdges = false;
  bool printEdgesPost = false;

  // Assume a 4kB Cache
  // with 16 Sets, associativity of 4 and Cachelines fitting two
  CacheType Cache = CacheType(16, 4, 128);
  StringRef EntryPoint = "main";
  unsigned int EntryAddress;
  unsigned int AddressCounter = 0b100000;
  // assume 8 Bit addressed 64 Bit instructions.
  std::map<unsigned int, Value *> Addr2Value;
  std::map<Value *, unsigned int> Value2Addr;

  AbstractCache AC;
  // TODO mark visit ed F's BB's and Inst's
  std::map<Function *, bool> VisitedFunctions;

  unsigned int stringRefToInt(StringRef SR) {
    unsigned int Length = SR.size();
    unsigned int ret = 0;
    unsigned int Count = 1;
    for (char C : SR) {
      unsigned int Factor = (unsigned int)pow(10, (Length - Count++));
      switch (C) {
      case '0':
        break;
      case '1':
        ret += Factor;
        break;
      case '2':
        ret += 2 * Factor;
        break;
      case '3':
        ret += 3 * Factor;
        break;
      case '4':
        ret += 4 * Factor;
        break;
      case '5':
        ret += 5 * Factor;
        break;
      case '6':
        ret += 6 * Factor;
        break;
      case '7':
        ret += 7 * Factor;
        break;
      case '8':
        ret += 8 * Factor;
        break;
      case '9':
        ret += 9 * Factor;
        break;
      default:
        errs() << "StringRef is not a decimal number";
      };
    }
    return ret;
  }

  // This method implements what the pass does
  void address_collector(Module &M) {
    for (Function &F : M) {
      if (F.getName().equals(EntryPoint)) {
        EntryAddress = AddressCounter;
        outs() << "Found main at Address: " << EntryAddress << " \n";
      }
      unsigned int InstCounter = 0;
      for (BasicBlock &BB : F) {
        Addr2Value[AddressCounter] = &BB;
        Value2Addr[&BB] = AddressCounter;
        AC.addEmptyNode(AddressCounter);
        AddressCounter += 1;
        InstCounter++;
        for (Instruction &Inst : BB) {
          AC.addEmptyNode(AddressCounter);
          Addr2Value[AddressCounter] = &Inst;
          Value2Addr[&Inst] = AddressCounter;
          AddressCounter += 1;
          InstCounter++;
        }
      }
    }
  }

  void address_printer(Function &F) {
    outs() << "F: " << Value2Addr[&F] << ".\n";
    for (BasicBlock &BB : F) {
      outs() << "-BB: " << Value2Addr[&BB] << "\n";
      for (Instruction &Inst : BB) {
        outs() << "--InstAddress:" << Value2Addr[&Inst] << "\n";
      }
    }
  }

  void init_edges(Function &F) {
    for (BasicBlock &BB : F) {
      // Collect Controll flow in F
      for (auto Pred : predecessors(&BB)) {
        AC.addEdge(Value2Addr[Pred], Value2Addr[&BB]);
        if (printEdges)
          outs() << Value2Addr[Pred] << " -> " << Value2Addr[&BB] << "\n";
      }
      Instruction *PrevInst = nullptr;
      for (Instruction &Inst : BB) {
        // Collect function Calls in F=main
        if (CallInst *Caller = dyn_cast<CallInst>(&Inst)) {
          Function *Callee = Caller->getCalledFunction();
          if (printEdges)
            outs() << "F: " << Callee->getName() << "\n"
                   << "Inst: " << Caller->getName() << "\n";
          if (Callee != NULL) {
            // Add edge on Function Call
            AC.addEdge(Value2Addr[&Inst],
                       Value2Addr[&Callee->getBasicBlockList()
                                       .front()
                                       .getInstList()
                                       .front()]);
            // Add edge on Function return
            AC.addEdge(
                Value2Addr
                    [&Callee->getBasicBlockList().back().getInstList().back()],
                Value2Addr[&Inst]);

            if (printEdges) {
              // Printing edge on Function Call
              outs() << Callee->getName() << ": ";
              outs() << Value2Addr[&Inst] << " -> "
                     << Value2Addr[&Callee->getBasicBlockList()
                                        .front()
                                        .getInstList()
                                        .front()]
                     << "\n";
              // Printing edge on Function return
              outs() << Callee->getName() << ": ";
              outs() << Value2Addr[&Callee->getBasicBlockList()
                                        .back()
                                        .getInstList()
                                        .back()]
                     << " -> " << Value2Addr[&Inst] << "\n";
            }

            // Resume CFG construction in called function
            if (VisitedFunctions.find(Callee) == VisitedFunctions.end()) {
              VisitedFunctions[Callee] = true;
              init_edges(*Callee);
            }
            PrevInst = nullptr;
            if (printEdges)
              outs() << "Back from " << Callee->getName() << "\n";
          }
        }
        if (PrevInst != nullptr) {
          AC.addEdge(Value2Addr[PrevInst], Value2Addr[&Inst]);
          if (printEdges)
            outs() << Value2Addr[PrevInst] << " -> " << Value2Addr[&Inst]
                   << "\n";
        }
        PrevInst = &Inst;
      }
    }
  }

  PreservedAnalyses run(Module &M, ModuleAnalysisManager &MAM) {
    FunctionAnalysisManager &FAM =
        MAM.getResult<FunctionAnalysisManagerModuleProxy>(M).getManager();

    address_collector(M);
    for (Function &F : M.getFunctionList()) {

      // Start iterating through CFG from entry point
      if (F.getName().equals(EntryPoint)) {
        init_edges(F);
      }

      if (printAddresses)
        address_printer(F);
      // ScalarEvolution &ScEv = FAM.getResult<ScalarEvolutionAnalysis>(F);
      // ScEv.print(outs());
      // LoopInfo &LI = FAM.getResult<LoopAnalysis>(F);
      //  for (Loop *L : LI) {
      //    outs() << "F: " << F.getName() << "\n";
      //    L->print(outs());
      //    outs() << "MaxBackEdgeTaken: ";
      //    ScEv.getConstantMaxBackedgeTakenCount(L)->print(outs());
      //    outs() << "\n";
      //  }
    }
    if (printEdgesPost)
      AC.dumpEdges();
    AC.fillAbstractCache();
    return PreservedAnalyses::all();
  }
};

} // namespace

//-----------------------------------------------------------------------------
// New PM Registration
//-----------------------------------------------------------------------------
llvm::PassPluginLibraryInfo getCacheAnalysisPassPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "CacheAnalysisPass", LLVM_VERSION_STRING,
          [](PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](StringRef Name, ModulePassManager &MPM,
                   ArrayRef<PassBuilder::PipelineElement>) {
                  if (Name == "lru-misses") {
                    MPM.addPass(CacheAnalysisPass());
                    return true; // only looks at CFG
                  }
                  return false; // Analysis pass.
                });
          }};
}

// This is the core interface for pass plugins. It guarantees that 'opt' will
// be able to recognize CacheAnalysisPass when added to the pass pipeline on
// the command line, i.e. via '-passes=lru-misses'
extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
  return getCacheAnalysisPassPluginInfo();
}
