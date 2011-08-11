#include <CPPOpenCLKernelSubroutine.h>




CPPOpenCLKernelSubroutine::CPPOpenCLKernelSubroutine (
    std::string const & subroutineName, 
    std::string const & userSubroutineName,
    SgScopeStatement * moduleScope, 
    CPPParallelLoop * parallelLoop,
    CPPReductionSubroutines * reductionSubroutines) :
  CPPKernelSubroutine (subroutineName, userSubroutineName, moduleScope,
      parallelLoop, reductionSubroutines)
{
  using SageInterface::addTextForUnparser;

  addTextForUnparser (subroutineHeaderStatement, "__kernel ", AstUnparseAttribute::e_before);
}
