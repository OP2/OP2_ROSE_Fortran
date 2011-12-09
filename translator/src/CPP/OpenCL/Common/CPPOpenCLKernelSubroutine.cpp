#include "CPPOpenCLKernelSubroutine.h"
#include "CPPOpenCLUserSubroutine.h"
#include "CPPReductionSubroutines.h"

CPPOpenCLKernelSubroutine::CPPOpenCLKernelSubroutine (
    SgScopeStatement * moduleScope, CPPOpenCLUserSubroutine * userSubroutine,
    CPPParallelLoop * parallelLoop,
    CPPReductionSubroutines * reductionSubroutines) :
  CPPKernelSubroutine (moduleScope, userSubroutine, parallelLoop)
{
  this->reductionSubroutines = reductionSubroutines;

  subroutineHeaderStatement->get_functionModifier ().setOpenclKernel ();
}
