#include <FortranOpenMPKernelSubroutine.h>

/*
 * ======================================================
 * Protected functions
 * ======================================================
 */

FortranOpenMPKernelSubroutine::FortranOpenMPKernelSubroutine (
    std::string const & subroutineName, std::string const & userSubroutineName,
    ParallelLoop * parallelLoop, SgScopeStatement * moduleScope) :
  FortranKernelSubroutine (subroutineName, userSubroutineName, parallelLoop)
{
  using SageBuilder::buildFunctionParameterList;
  using SageBuilder::buildVoidType;
  using SageBuilder::buildProcedureHeaderStatement;
  using SageInterface::appendStatement;

  formalParameters = buildFunctionParameterList ();

  subroutineHeaderStatement = buildProcedureHeaderStatement (
      this->subroutineName.c_str (), buildVoidType (), formalParameters,
      SgProcedureHeaderStatement::e_subroutine_subprogram_kind, moduleScope);

  appendStatement (subroutineHeaderStatement, moduleScope);

  subroutineScope = subroutineHeaderStatement->get_definition ()->get_body ();
}
