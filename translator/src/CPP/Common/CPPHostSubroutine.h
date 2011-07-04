#ifndef CPP_HOST_SUBROUTINE_H
#define CPP_HOST_SUBROUTINE_H

#include <HostSubroutine.h>
#include <CPPParallelLoop.h>
#include <CPPProgramDeclarationsAndDefinitions.h>

class CPPHostSubroutine: public HostSubroutine <SgFunctionDeclaration,
    CPPProgramDeclarationsAndDefinitions>
{

  private:

    virtual SgStatement *
    createKernelFunctionCallStatement ();

    virtual void
    createReductionEpilogueStatements ();

    virtual void
    createReductionPrologueStatements ();

    virtual void
    createReductionLocalVariableDeclarations ();

    virtual void
    createStatements ();

    virtual void
    createLocalVariableDeclarations ();

    virtual void
    createFormalParameterDeclarations ();

  protected:

    CPPHostSubroutine (std::string const & subroutineName,
        std::string const & userSubroutineName,
        std::string const & kernelSubroutineName,
        SgScopeStatement * moduleScope, CPPParallelLoop * parallelLoop) :
      HostSubroutine <SgFunctionDeclaration,
          CPPProgramDeclarationsAndDefinitions> (subroutineName,
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
    }
};

#endif
