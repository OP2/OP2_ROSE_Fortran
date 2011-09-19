#ifndef FORTRAN_CUDA_USER_SUBROUTINE_H
#define FORTRAN_CUDA_USER_SUBROUTINE_H

#include <UserSubroutine.h>
#include <FortranParallelLoop.h>
#include <FortranProgramDeclarationsAndDefinitions.h>

class FortranCUDAUserSubroutine: public UserSubroutine <
    SgProcedureHeaderStatement, FortranProgramDeclarationsAndDefinitions>
{
  private:

    SgProcedureHeaderStatement * originalSubroutine;

  private:

    void
    forceOutputOfCodeToFile ();

    void
    findOriginalSubroutine ();

    virtual void
    createStatements ();

    virtual void
    createLocalVariableDeclarations ();

    virtual void
    createFormalParameterDeclarations ();

  public:

    FortranCUDAUserSubroutine (std::string const & subroutineName,
        SgScopeStatement * moduleScope,
        FortranProgramDeclarationsAndDefinitions * declarations,
        FortranParallelLoop * parallelLoop);
};

#endif
