#include <CPPOpenCLKernelSubroutine.h>




CPPOpenCLKernelSubroutine::CPPOpenCLKernelSubroutine (
    std::string const & subroutineName, 
    std::string const & userSubroutineName,
    CPPParallelLoop * parallelLoop,
    SgScopeStatement * moduleScope, 
    CPPReductionSubroutines * reductionSubroutines,
    CPPOpDatDimensionsDeclaration * opDatDimensionsDeclaration) :
  CPPKernelSubroutine (
      subroutineName, 
      userSubroutineName, 
      parallelLoop,
      moduleScope,
      reductionSubroutines),
  opDatDimensionsDeclaration( opDatDimensionsDeclaration )
{
  using SageInterface::addTextForUnparser;

  addTextForUnparser (subroutineHeaderStatement, "__kernel ", AstUnparseAttribute::e_before);
}
