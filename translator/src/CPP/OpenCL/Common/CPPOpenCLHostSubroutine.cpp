#include "CPPOpenCLHostSubroutine.h"
#include "CPPOpenCLKernelSubroutine.h"
#include "CPPModuleDeclarations.h"

void
CPPOpenCLHostSubroutine::createReductionEpilogueStatements ()
{
}

void
CPPOpenCLHostSubroutine::createReductionPrologueStatements ()
{
}

void
CPPOpenCLHostSubroutine::createReductionDeclarations ()
{
}

CPPOpenCLHostSubroutine::CPPOpenCLHostSubroutine (
    SgScopeStatement * moduleScope,
    CPPOpenCLKernelSubroutine * calleeSubroutine,
    CPPParallelLoop * parallelLoop, CPPModuleDeclarations * moduleDeclarations) :
  CPPHostSubroutine (moduleScope, calleeSubroutine, parallelLoop),
      moduleDeclarations (moduleDeclarations)
{
  variableDeclarations->addVisibilityToSymbolsFromOuterScope (
      moduleDeclarations->getDeclarations ());
}
