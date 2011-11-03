#include <CPPCUDAUserSubroutine.h>
#include <CPPParallelLoop.h>
#include <CPPProgramDeclarationsAndDefinitions.h>

CPPCUDAUserSubroutine::CPPCUDAUserSubroutine (SgScopeStatement * moduleScope,
    CPPParallelLoop * parallelLoop,
    CPPProgramDeclarationsAndDefinitions * declarations) :
  CPPUserSubroutine (moduleScope, parallelLoop, declarations)
{
  subroutineHeaderStatement->get_functionModifier ().setCudaDevice ();
}
