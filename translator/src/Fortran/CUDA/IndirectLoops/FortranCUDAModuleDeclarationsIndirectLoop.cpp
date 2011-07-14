#include <FortranCUDAModuleDeclarationsIndirectLoop.h>

FortranCUDAModuleDeclarationsIndirectLoop::FortranCUDAModuleDeclarationsIndirectLoop (
    std::string const & userSubroutineName, FortranParallelLoop * parallelLoop,
    SgScopeStatement * moduleScope) :
  FortranCUDAModuleDeclarations (userSubroutineName, parallelLoop, moduleScope)
{
}
