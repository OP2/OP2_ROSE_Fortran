#include <FortranKernelSubroutine.h>

/*
 * ======================================================
 * Protected functions
 * ======================================================
 */

FortranKernelSubroutine::FortranKernelSubroutine (
    std::string const & subroutineName, std::string const & userSubroutineName,
    ParallelLoop * parallelLoop, SgScopeStatement * moduleScope) :
  FortranSubroutine (), KernelSubroutine (subroutineName, userSubroutineName,
      parallelLoop)
{
  using SageBuilder::buildVoidType;
  using SageBuilder::buildProcedureHeaderStatement;
  using SageInterface::appendStatement;

  subroutineHeaderStatement = buildProcedureHeaderStatement (
      this->subroutineName.c_str (), buildVoidType (), formalParameters,
      SgProcedureHeaderStatement::e_subroutine_subprogram_kind, moduleScope);

  subroutineScope = subroutineHeaderStatement->get_definition ()->get_body ();

  appendStatement (subroutineHeaderStatement, moduleScope);
}
