#include<FortranOpenMPHostSubroutineDirectLoop.h>

FortranOpenMPHostSubroutineDirectLoop::FortranOpenMPHostSubroutineDirectLoop (
    std::string const & subroutineName, Subroutine * userSubroutine,
    Subroutine * kernelSubroutine, ParallelLoop * parallelLoop,
    SgScopeStatement * moduleScope) :
  FortranOpenMPHostSubroutine (subroutineName, userSubroutine,
      kernelSubroutine, parallelLoop, moduleScope)
{
  createFormalParameterDeclarations ();
}
