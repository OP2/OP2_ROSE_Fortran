#ifndef CPP_HOST_SUBROUTINE_H
#define CPP_HOST_SUBROUTINE_H

#include <HostSubroutine.h>
#include <CPPParallelLoop.h>

class CPPHostSubroutine: 
  public HostSubroutine <SgFunctionDeclaration>
{

  protected:


    virtual void
    createFormalParameterDeclarations ();


    CPPHostSubroutine (std::string const & subroutineName,
        std::string const & userSubroutineName,
        std::string const & kernelSubroutineName,
        CPPParallelLoop * parallelLoop,
        SgScopeStatement * moduleScope) :
      HostSubroutine <SgFunctionDeclaration> (
          subroutineName,
          userSubroutineName, 
          kernelSubroutineName, 
          parallelLoop)
    {
      using SageBuilder::buildVoidType;
      using SageBuilder::buildDefiningFunctionDeclaration;
      using SageInterface::appendStatement;

      subroutineHeaderStatement = buildDefiningFunctionDeclaration (
          this->subroutineName.c_str (), 
          buildVoidType (), 
          formalParameters,
          moduleScope);

      subroutineScope
          = subroutineHeaderStatement->get_definition ()->get_body ();

      appendStatement (subroutineHeaderStatement, moduleScope);
    }
};

#endif
