#ifndef FORTRAN_CUDA_USER_SUBROUTINE_H
#define FORTRAN_CUDA_USER_SUBROUTINE_H

#include <UserSubroutine.h>
#include <CPPInitialiseConstantsSubroutine.h>
#include <CPPParallelLoop.h>

class CPPOpenCLUserSubroutine:
  public UserSubroutine <SgFunctionDeclaration, CPPProgramDeclarationsAndDefinitions>
{
  private:

    FortranInitialiseConstantsSubroutine * initialiseConstantsSubroutine;

  private:

    void
    patchReferencesToConstants ();

    virtual void
    createStatements ();

    virtual void
    createLocalVariableDeclarations ();

    virtual void
    createFormalParameterDeclarations ();

  public:

        CPPOpenCLUserSubroutine (
            std::string const & subroutineName,
            SgScopeStatement * moduleScope,
            CPPInitialiseConstantsSubroutine * initialiseConstantsSubroutine,
            CPPProgramDeclarationsAndDefinitions * declarations,
            CPPParallelLoop * parallelLoop);
};

#endif
