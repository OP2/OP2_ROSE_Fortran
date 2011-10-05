#include <FortranKernelSubroutine.h>
#include <FortranStatementsAndExpressionsBuilder.h>
#include <FortranParallelLoop.h>

FortranKernelSubroutine::FortranKernelSubroutine (
    std::string const & subroutineName, std::string const & userSubroutineName,
    FortranParallelLoop * parallelLoop, SgScopeStatement * moduleScope) :
  KernelSubroutine <SgProcedureHeaderStatement> (subroutineName,
      userSubroutineName, parallelLoop)
{
  using SageBuilder::buildVoidType;
  using SageBuilder::buildProcedureHeaderStatement;
  using SageInterface::appendStatement;

  subroutineHeaderStatement = buildProcedureHeaderStatement (
      this->subroutineName.c_str (), buildVoidType (), formalParameters,
      SgProcedureHeaderStatement::e_subroutine_subprogram_kind, moduleScope);

  subroutineScope = subroutineHeaderStatement->get_definition ()->get_body ();

  appendStatement (subroutineHeaderStatement, moduleScope);

  appendStatement (
      FortranStatementsAndExpressionsBuilder::buildImplicitNoneStatement (),
      subroutineScope);
}
