#ifndef FORTRAN_HOST_SUBROUTINE_H
#define FORTRAN_HOST_SUBROUTINE_H

#include <HostSubroutine.h>
#include <FortranParallelLoop.h>
#include <FortranStatementsAndExpressionsBuilder.h>

class FortranHostSubroutine: public HostSubroutine <SgProcedureHeaderStatement>
{
  protected:

    virtual SgBasicBlock *
    createTransferOpDatStatements () = 0;

    virtual void
    createFormalParameterDeclarations ();

    FortranHostSubroutine (std::string const & subroutineName,
        std::string const & userSubroutineName,
        std::string const & kernelSubroutineName,
        FortranParallelLoop * parallelLoop, SgScopeStatement * moduleScope) :
      HostSubroutine <SgProcedureHeaderStatement> (subroutineName,
          userSubroutineName, kernelSubroutineName, parallelLoop)
    {
      using SageBuilder::buildVoidType;
      using SageBuilder::buildProcedureHeaderStatement;
      using SageInterface::appendStatement;

      subroutineHeaderStatement
          = buildProcedureHeaderStatement (this->subroutineName.c_str (),
              buildVoidType (), formalParameters,
              SgProcedureHeaderStatement::e_subroutine_subprogram_kind,
              moduleScope);

      subroutineScope
          = subroutineHeaderStatement->get_definition ()->get_body ();

      appendStatement (subroutineHeaderStatement, moduleScope);

      appendStatement (
          FortranStatementsAndExpressionsBuilder::buildImplicitNoneStatement (),
          subroutineScope);
    }
};

#endif
