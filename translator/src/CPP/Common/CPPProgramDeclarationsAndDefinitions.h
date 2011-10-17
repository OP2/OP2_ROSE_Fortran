#pragma once
#ifndef CPP_PROGRAM_DECLARATIONS_AND_DEFINITIONS_H
#define CPP_PROGRAM_DECLARATIONS_AND_DEFINITIONS_H

#include <ProgramDeclarationsAndDefinitions.h>

class CPPParallelLoop;

class CPPProgramDeclarationsAndDefinitions: public ProgramDeclarationsAndDefinitions <
    SgFunctionDeclaration> ,
    public AstSimpleProcessing
{
  private:

    std::map <unsigned int, std::string> opAccessDescriptors;

  private:

    void
    setOpGblProperties (CPPParallelLoop * parallelLoop,
        std::string const & variableName, int OP_DAT_ArgumentGroup);

    void
    setOpDatProperties (CPPParallelLoop * parallelLoop,
        std::string const & variableName, int OP_DAT_ArgumentGroup);

    void
    setParallelLoopMappingDescriptor (CPPParallelLoop * parallelLoop,
        std::string const & mappingValue, unsigned int OP_DAT_ArgumentGroup);

    void
    setParallelLoopAccessDescriptor (CPPParallelLoop * parallelLoop,
        SgExprListExp * actualArguments, unsigned int OP_DAT_ArgumentGroup,
        unsigned int argumentPosition);

    void
    handleImperialOpGblArgument (CPPParallelLoop * parallelLoop,
        SgExprListExp * actualArguments, unsigned int OP_DAT_ArgumentGroup);

    void
    handleOxfordOpGblArgument (CPPParallelLoop * parallelLoop,
        SgExprListExp * actualArguments, unsigned int OP_DAT_ArgumentGroup);

    void
    handleImperialOpDatArgument (CPPParallelLoop * parallelLoop,
        SgExprListExp * actualArguments, unsigned int OP_DAT_ArgumentGroup);

    void
    handleOxfordOpDatArgument (CPPParallelLoop * parallelLoop,
        SgExprListExp * actualArguments, unsigned int OP_DAT_ArgumentGroup);

    void
    analyseParallelLoopArguments (CPPParallelLoop * parallelLoop,
        SgExprListExp * actualArguments);

    void
    detectAndHandleOP2Definition (SgVariableDeclaration * variableDeclaration,
        std::string const variableName, SgTypedefType * typeDefinition);

    virtual void
    visit (SgNode * node);

  public:

    CPPProgramDeclarationsAndDefinitions (SgProject * project);
};

#endif
