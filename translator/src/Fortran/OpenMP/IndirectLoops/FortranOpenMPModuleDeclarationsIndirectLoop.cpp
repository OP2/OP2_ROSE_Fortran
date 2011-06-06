#include <FortranOpenMPModuleDeclarationsIndirectLoop.h>

FortranOpenMPModuleDeclarationsIndirectLoop::FortranOpenMPModuleDeclarationsIndirectLoop (
    std::string const & userSubroutineName, ParallelLoop * parallelLoop,
    SgScopeStatement * moduleScope) :
  FortranOpenMPModuleDeclarations (userSubroutineName, parallelLoop,
      moduleScope)
{
}
