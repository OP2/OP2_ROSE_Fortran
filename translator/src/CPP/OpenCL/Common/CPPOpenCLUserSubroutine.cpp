#include <CPPOpenCLUserSubroutine.h>
#include <CPPParallelLoop.h>
#include <CPPProgramDeclarationsAndDefinitions.h>

CPPOpenCLUserSubroutine::CPPOpenCLUserSubroutine (
    SgScopeStatement * moduleScope, CPPParallelLoop * parallelLoop,
    CPPProgramDeclarationsAndDefinitions * declarations) :
  CPPUserSubroutine (moduleScope, parallelLoop, declarations)
{
  subroutineHeaderStatement->get_functionModifier ().setInline ();
}
