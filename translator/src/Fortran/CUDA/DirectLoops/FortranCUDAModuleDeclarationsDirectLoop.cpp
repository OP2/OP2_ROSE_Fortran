#include <FortranCUDAModuleDeclarationsDirectLoop.h>

FortranCUDAModuleDeclarationsDirectLoop::FortranCUDAModuleDeclarationsDirectLoop (
    std::string const & userSubroutineName, FortranParallelLoop * parallelLoop,
    SgScopeStatement * moduleScope,
    FortranCUDADataSizesDeclarationDirectLoop * dataSizesDeclaration) :
  FortranCUDAModuleDeclarations (userSubroutineName, parallelLoop, moduleScope,
      dataSizesDeclaration)
{
}
