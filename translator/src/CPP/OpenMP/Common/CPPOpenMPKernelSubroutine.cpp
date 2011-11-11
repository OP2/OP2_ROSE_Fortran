#include <CPPOpenMPKernelSubroutine.h>
#include <CPPUserSubroutine.h>

CPPOpenMPKernelSubroutine::CPPOpenMPKernelSubroutine (
    SgScopeStatement * moduleScope, CPPUserSubroutine * userSubroutine,
    CPPParallelLoop * parallelLoop) :
  CPPKernelSubroutine (moduleScope, userSubroutine, parallelLoop)
{

}
