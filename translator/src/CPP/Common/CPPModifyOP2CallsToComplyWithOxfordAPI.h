#pragma once
#ifndef CPP_MODIFY_OP2_CALLS_TO_COMPLY_WITH_OXFORD_API_H
#define CPP_MODIFY_OP2_CALLS_TO_COMPLY_WITH_OXFORD_API_H

#include <rose.h>

class CPPProgramDeclarationsAndDefinitions;

class CPPModifyOP2CallsToComplyWithOxfordAPI: public AstSimpleProcessing
{
  private:

    CPPProgramDeclarationsAndDefinitions * declarations;

  private:

    void
    patchOpArgGblCall (SgExpressionPtrList & actualArguments);

    void
    patchOpArgDatCall (SgExpressionPtrList & actualArguments);

    void
    patchOpParLoopCall (SgExpressionPtrList & actualArguments);

    void
    patchOpDeclareConstCall (SgExpressionPtrList & actualArguments);

    void
    patchOpDeclareDatCall (SgVariableDeclaration * variableDeclaration,
        std::string const variableName);

    void
    patchOpDeclareMapCall (SgVariableDeclaration * variableDeclaration,
        std::string const variableName);

    void
    patchOpDeclareSetCall (SgVariableDeclaration * variableDeclaration,
        std::string const variableName);

    virtual void
    visit (SgNode * node);

  public:

    CPPModifyOP2CallsToComplyWithOxfordAPI (SgProject * project,
        CPPProgramDeclarationsAndDefinitions * declarations);
};

#endif
