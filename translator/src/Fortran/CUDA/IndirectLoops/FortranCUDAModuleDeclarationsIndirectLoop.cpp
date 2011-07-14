#include <FortranCUDAModuleDeclarationsIndirectLoop.h>

FortranCUDAModuleDeclarationsIndirectLoop::FortranCUDAModuleDeclarationsIndirectLoop (
    std::string const & userSubroutineName, FortranParallelLoop * parallelLoop,
    SgScopeStatement * moduleScope,
    FortranCUDADataSizesDeclarationIndirectLoop * dataSizesDeclaration) :
  FortranCUDAModuleDeclarations (userSubroutineName, parallelLoop, moduleScope,
      dataSizesDeclaration)
{
}
