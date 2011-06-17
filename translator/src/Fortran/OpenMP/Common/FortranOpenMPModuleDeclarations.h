#ifndef FORTRAN_CUDA_OPENMP_MODULE_DECLARATIONS_H
#define FORTRAN_CUDA_OPENMP_MODULE_DECLARATIONS_H

#include <string>
#include <rose.h>
#include <ParallelLoop.h>
#include <VariableDeclarations.h>

class FortranOpenMPModuleDeclarations
{

  private:

    /*
     * ======================================================
     * The name of the user subroutine
     * ======================================================
     */
    std::string userSubroutineName;

    /*
     * ======================================================
     * Variables declared at module scope level
     * ======================================================
     */
    VariableDeclarations * moduleDeclarations;

    /*
     * ======================================================
     * The parallel loop
     * ======================================================
     */
    ParallelLoop * parallelLoop;

    /*
     * ======================================================
     * The module scope
     * ======================================================
     */
    SgScopeStatement * moduleScope;

  protected:

    void
    createOPDATDeclarations ();

    void
    createFirstExecutionBooleanDeclaration ();

    std::string
    getFirstExecutionBooleanVariableName ();

    FortranOpenMPModuleDeclarations (std::string const & userSubroutineName,
        ParallelLoop * parallelLoop, SgScopeStatement * moduleScope);

  public:

    SgVariableDeclaration *
    getGlobalOPDATDeclaration (unsigned int OP_DAT_ArgumentGroup);

    SgVariableDeclaration *
    getFirstExecutionBooleanDeclaration ();
};

#endif
