#pragma once
#ifndef CPP_USER_SUBROUTINE_H
#define CPP_USER_SUBROUTINE_H

#include <UserSubroutine.h>

class CPPParallelLoop;
class CPPProgramDeclarationsAndDefinitions;

class CPPUserSubroutine: public UserSubroutine <SgFunctionDeclaration,
    CPPProgramDeclarationsAndDefinitions> , public AstSimpleProcessing
{
  private:

    std::vector <std::string> referencedOpDeclConsts;

  protected:

    virtual void
    visit (SgNode * node);

    virtual void
    createStatements ();

    virtual void
    createLocalVariableDeclarations ();

    virtual void
    createFormalParameterDeclarations ();

  public:

    std::vector <std::string>::const_iterator
    firstOpConstReference ()
    {
      return referencedOpDeclConsts.begin ();
    }

    std::vector <std::string>::const_iterator
    lastOpConstReference ()
    {
      return referencedOpDeclConsts.end ();
    }

    CPPUserSubroutine (SgScopeStatement * moduleScope,
        CPPParallelLoop * parallelLoop,
        CPPProgramDeclarationsAndDefinitions * declarations);
};

#endif
