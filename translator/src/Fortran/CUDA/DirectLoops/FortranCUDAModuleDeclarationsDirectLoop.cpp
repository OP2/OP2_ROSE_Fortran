#include <FortranCUDAModuleDeclarationsDirectLoop.h>

FortranCUDAModuleDeclarationsDirectLoop::FortranCUDAModuleDeclarationsDirectLoop (
    std::string const & userSubroutineName, FortranParallelLoop * parallelLoop,
    SgScopeStatement * moduleScope) :
  FortranCUDAModuleDeclarations (userSubroutineName, parallelLoop, moduleScope)
{
}
