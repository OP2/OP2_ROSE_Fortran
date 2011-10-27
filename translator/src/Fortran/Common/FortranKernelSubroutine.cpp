#include <FortranKernelSubroutine.h>
#include <FortranStatementsAndExpressionsBuilder.h>
#include <FortranParallelLoop.h>

FortranKernelSubroutine::FortranKernelSubroutine (
    SgScopeStatement * moduleScope,
    Subroutine <SgProcedureHeaderStatement> * calleeSubroutine,
    FortranParallelLoop * parallelLoop) :
      KernelSubroutine <SgProcedureHeaderStatement> (calleeSubroutine,
          parallelLoop)
{
  using namespace SageBuilder;
  using namespace SageInterface;

  subroutineHeaderStatement = buildProcedureHeaderStatement (
      this->subroutineName.c_str (), buildVoidType (), formalParameters,
      SgProcedureHeaderStatement::e_subroutine_subprogram_kind, moduleScope);

  subroutineScope = subroutineHeaderStatement->get_definition ()->get_body ();

  appendStatement (subroutineHeaderStatement, moduleScope);

  appendStatement (
      FortranStatementsAndExpressionsBuilder::buildImplicitNoneStatement (),
      subroutineScope);
}
