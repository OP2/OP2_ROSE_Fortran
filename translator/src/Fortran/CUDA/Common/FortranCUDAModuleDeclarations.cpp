#include <FortranCUDAModuleDeclarations.h>
#include <FortranParallelLoop.h>
#include <FortranCUDAOpDatCardinalitiesDeclaration.h>
#include <FortranOpDatDimensionsDeclaration.h>

FortranCUDAModuleDeclarations::FortranCUDAModuleDeclarations (
    FortranParallelLoop * parallelLoop, SgScopeStatement * moduleScope,
    FortranCUDAOpDatCardinalitiesDeclaration * dataSizesDeclaration,
    FortranOpDatDimensionsDeclaration * dimensionsDeclaration) :
  FortranModuleDeclarations (parallelLoop, moduleScope), dataSizesDeclaration (
      dataSizesDeclaration), dimensionsDeclaration (dimensionsDeclaration)
{
}
