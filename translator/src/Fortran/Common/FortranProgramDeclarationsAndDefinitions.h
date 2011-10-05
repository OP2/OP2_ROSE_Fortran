/*
 * Written by Adam Betts and Carlo Bertolli
 */

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

    std::map <std::string, std::string> fileNameToModuleName;

  private:

    void
    handleOpGblDeclaration (FortranParallelLoop * parallelLoop,
        std::string const & variableName, int opDatArgumentGroup);

    void
    handleOpDatDeclaration (FortranParallelLoop * parallelLoop,
        std::string const & variableName, int opDatArgumentGroup);

    void
    setParallelLoopAccessDescriptor (FortranParallelLoop * parallelLoop,
        SgExprListExp * actualArguments,
        unsigned int OP_DAT_ArgumentGroup, unsigned int argumentPosition);

    void
    analyseParallelLoopArguments (FortranParallelLoop * parallelLoop,
        SgExprListExp * actualArguments);

    virtual void
    visit (SgNode * node);

  public:

    std::string const &
    getModuleNameForFile (std::string fileName)
    {
      return fileNameToModuleName[fileName];
    }

    std::string const &
    getFileNameForSubroutine (std::string subroutineName)
    {
      return subroutineToFileName[subroutineName];
    }

    FortranProgramDeclarationsAndDefinitions (SgProject * project);
};

#endif
