#include <CPPOpenCLKernelSubroutine.h>

void
CPPOpenCLKernelSubroutine::createReductionLoopStatements ()
{
  //FIXME
}

void
CPPOpenCLKernelSubroutine::createLocalThreadDeclarations ()
{
  //FIXME
}

void
CPPOpenCLKernelSubroutine::createAutoSharedDeclaration ()
{
  //FIXME
}

CPPOpenCLKernelSubroutine::CPPOpenCLKernelSubroutine (
    std::string const & subroutineName, std::string const & userSubroutineName,
    CPPParallelLoop * parallelLoop, SgScopeStatement * moduleScope,
    CPPReductionSubroutines * reductionSubroutines,
    CPPOpDatDimensionsDeclaration * opDatDimensionsDeclaration) :
  CPPKernelSubroutine (subroutineName, userSubroutineName, moduleScope,
      parallelLoop), opDatDimensionsDeclaration (opDatDimensionsDeclaration)
{
  using SageInterface::addTextForUnparser;

  addTextForUnparser (subroutineHeaderStatement, "__kernel ",
      AstUnparseAttribute::e_before);
}
