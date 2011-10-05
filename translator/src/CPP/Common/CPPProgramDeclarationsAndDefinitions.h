#pragma once
#ifndef CPP_PROGRAM_DECLARATIONS_AND_DEFINITIONS_H
#define CPP_PROGRAM_DECLARATIONS_AND_DEFINITIONS_H

#include <ProgramDeclarationsAndDefinitions.h>

class CPPProgramDeclarationsAndDefinitions: public ProgramDeclarationsAndDefinitions <
    SgFunctionDeclaration> ,
    public AstSimpleProcessing
{
  private:

    void
    detectAndHandleOP2Definition (SgVariableDeclaration * variableDeclaration,
        std::string const variableName, SgTypedefType * typeDefinition);

    virtual void
    visit (SgNode * node);

  public:

    CPPProgramDeclarationsAndDefinitions (SgProject * project);
};

#endif
