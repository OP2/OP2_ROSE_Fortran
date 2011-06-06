#include <FortranOpenMPHostSubroutine.h>
#include <CommonNamespaces.h>

/*
 * ======================================================
 * Protected functions
 * ======================================================
 */

FortranOpenMPHostSubroutine::FortranOpenMPHostSubroutine (
    std::string const & subroutineName, std::string const & userSubroutineName,
    std::string const & kernelSubroutineName, ParallelLoop * parallelLoop,
    SgScopeStatement * moduleScope) :
  FortranHostSubroutine (subroutineName, userSubroutineName,
      kernelSubroutineName, parallelLoop)
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
