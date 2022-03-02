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
#include "llvm/ADT/MapVector.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/IR/AbstractCallSite.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Pass.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"

#include <cstdlib>
#include <llvm/Analysis/LoopInfo.h>

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

unsigned int getTypeSize(Type &T) {
  unsigned int Ret = 0;
  switch (T.getTypeID()) {
  case Type::TypeID::ArrayTyID:
    Ret = 0;
  case Type::TypeID::BFloatTyID:
    Ret = 32;
  case Type::TypeID::FloatTyID:
    Ret = 32;
  case Type::TypeID::DoubleTyID:
    Ret = 64;
  case Type::TypeID::FixedVectorTyID:
    Ret = 0; // TODO
  case Type::TypeID::FP128TyID:
    Ret = 128;
  case Type::TypeID::FunctionTyID:
    Ret = 0; // TODO
  case Type::TypeID::HalfTyID:
    Ret = 0; // TODO
  case Type::TypeID::IntegerTyID:
    Ret = T.getIntegerBitWidth();
  case Type::TypeID::LabelTyID:
    Ret = 0; // TODO
  case Type::TypeID::MetadataTyID:
    Ret = 0; // TODO
  case Type::TypeID::PointerTyID:
    Ret = 64; // Assume 64bit Architecture get it from target
  case Type::TypeID::PPC_FP128TyID:
    Ret = 128;
  case Type::TypeID::ScalableVectorTyID:
    Ret = 0; // TODO
  case Type::TypeID::StructTyID:
    Ret = 0; // TODO
  case Type::TypeID::TokenTyID:
    Ret = 0; // TODO
  case Type::TypeID::VoidTyID:
    Ret = 0; // TODO
  case Type::TypeID::X86_AMXTyID:
    Ret = 0; // TODO
  case Type::TypeID::X86_FP80TyID:
    Ret = 0; // TODO
  case Type::TypeID::X86_MMXTyID:
    Ret = 0; // TODO
  }
  if (Ret == 0) {
    errs() << "encountered Unhandeled DataType. aborting!";
    exit(EXIT_FAILURE);
  }
  // should not reach here
  return Ret;
}

void collect_globals(Module &M) {
  DataLayout DL = M.getDataLayout();
  Module::GlobalListType &GlobalsList = M.getGlobalList();
  for (GlobalVariable &G : GlobalsList) {
    outs() << "G: " << G.getName() << "\n";
    outs() << "  EleType: "
           << typeToName(G.getType()->getElementType()->getTypeID()) << "\n";

    switch (G.getType()->getElementType()->getTypeID()) {
    case Type::TypeID::IntegerTyID:
      outs() << "size: " << G.getType()->getElementType()->getIntegerBitWidth()
             << "\n";
    }
  }
}

// This method implements what the pass does
void function_visitor(Function &F) {
  outs() << "Hello from: " << F.getName() << "\n";
  // outs() << "  number of arguments: " << F.arg_size() << "\n";
}

// New PM implementation
struct CacheAnalysisPass : PassInfoMixin<CacheAnalysisPass> {

  PreservedAnalyses run(Module &M, ModuleAnalysisManager &MAM) {
    FunctionAnalysisManager &FAM =
        MAM.getResult<FunctionAnalysisManagerModuleProxy>(M).getManager();

    for (Function &F : M.getFunctionList()) {
      function_visitor(F);
      ScalarEvolution &ScEv = FAM.getResult<ScalarEvolutionAnalysis>(F);
      LoopInfo &LI = FAM.getResult<LoopAnalysis>(F);
      for (auto &L : LI) {
        L->print(outs());
        outs() << "MaxBackEdgeTaken: ";
        ScEv.getConstantMaxBackedgeTakenCount(L)->print(outs());
        outs() << "\n";
      }
      // LI.print(outs());
      // ScEv.print(outs());
    }
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
                    return true;
                  }
                  return false;
                });
          }};
}

// This is the core interface for pass plugins. It guarantees that 'opt' will
// be able to recognize CacheAnalysisPass when added to the pass pipeline on the
// command line, i.e. via '-passes=lru-misses'
extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
  return getCacheAnalysisPassPluginInfo();
}
