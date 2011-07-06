#ifndef CPP_KERNEL_SUBROUTINE_H
#define CPP_KERNEL_SUBROUTINE_H

#include <KernelSubroutine.h>
#include <CPPParallelLoop.h>

class CPPKernelSubroutine: public KernelSubroutine <SgFunctionDeclaration,
    CPPProgramDeclarationsAndDefinitions>
{

  protected:

    CPPKernelSubroutine::CPPKernelSubroutine (
        std::string const & subroutineName,
        std::string const & userSubroutineName, SgScopeStatement * moduleScope,
        CPPParallelLoop * parallelLoop) :
      KernelSubroutine <SgFunctionDeclaration,
          CPPProgramDeclarationsAndDefinitions> (subroutineName,
          userSubroutineName, parallelLoop)
    {
      using SageBuilder::buildVoidType;
      using SageBuilder::buildDefiningFunctionDeclaration;
      using SageInterface::appendStatement;

      subroutineHeaderStatement = buildDefiningFunctionDeclaration (
          this->subroutineName.c_str (), buildVoidType (), formalParameters,
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
