#pragma once
#ifndef FORTRAN_PROGRAM_DECLARATIONS_AND_DEFINITIONS_H
#define FORTRAN_PROGRAM_DECLARATIONS_AND_DEFINITIONS_H

#include <ProgramDeclarationsAndDefinitions.h>

class FortranParallelLoop;

class FortranProgramDeclarationsAndDefinitions: public ProgramDeclarationsAndDefinitions <
    SgProcedureHeaderStatement> ,
    public AstSimpleProcessing
{
  private:

    std::map <std::string, std::string> subroutineToFileName;

    std::map <std::string, std::vector <std::string> > moduleNameToSubroutines;

    std::map <std::string, std::vector <std::string> > fileNameToModuleNames;

    std::map <std::string, std::string> moduleNameToFileName;

    std::string currentModuleName;

  private:

    void
    setOpGblProperties (FortranParallelLoop * parallelLoop,
        std::string const & variableName, int opDatArgumentGroup);

    void
    setOpDatProperties (FortranParallelLoop * parallelLoop,
        std::string const & variableName, int opDatArgumentGroup);

    void
    setParallelLoopAccessDescriptor (FortranParallelLoop * parallelLoop,
        SgExprListExp * actualArguments, unsigned int OP_DAT_ArgumentGroup,
        unsigned int argumentPosition);

    void
    analyseParallelLoopArguments (FortranParallelLoop * parallelLoop,
        SgExprListExp * actualArguments);

    virtual void
    visit (SgNode * node);

    bool
    checkModuleExists (std::string const & moduleName);

  public:

    std::string const &
    getFileNameForModule (std::string const & moduleName);

    std::string const &
    getFileNameForSubroutine (std::string const & subroutineName);

    FortranProgramDeclarationsAndDefinitions (SgProject * project);
};

#endif
