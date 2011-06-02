#ifndef FORTRAN_CUDA_OPENMP_MODULE_DECLARATIONS_H
#define FORTRAN_CUDA_OPENMP_MODULE_DECLARATIONS_H

#include <string>
#include <rose.h>
#include <ParallelLoop.h>

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
    std::map <std::string, SgVariableDeclaration *> moduleDeclarations;

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

  private:

    static std::string
    getOPDATName (unsigned int OP_DAT_ArgumentGroup);

    void
    createOPDATDeclarations ();

    void
    createFirstExecutionBooleanDeclaration ();

    std::string
    getFirstExecutionBooleanVariableName ();

  public:

    SgVarRefExp *
    getReferenceToFirstExecutionBoolean ();

    FortranOpenMPModuleDeclarations (std::string const & userSubroutineName,
        ParallelLoop * parallelLoop, SgScopeStatement * moduleScope);

};

#endif
