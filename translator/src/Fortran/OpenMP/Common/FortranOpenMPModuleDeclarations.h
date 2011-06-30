#ifndef FORTRAN_CUDA_OPENMP_MODULE_DECLARATIONS_H
#define FORTRAN_CUDA_OPENMP_MODULE_DECLARATIONS_H

#include <string>
#include <rose.h>
#include <FortranParallelLoop.h>
#include <VariableDeclarations.h>

class FortranOpenMPModuleDeclarations
{

  protected:

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
    FortranParallelLoop * parallelLoop;

    /*
     * ======================================================
     * The module scope
     * ======================================================
     */
    SgScopeStatement * moduleScope;

  protected:

    void
    createOpDatDeclarations ();

    void
    createFirstExecutionBooleanDeclaration ();

    std::string
    getFirstExecutionBooleanVariableName ();

    virtual
    ~FortranOpenMPModuleDeclarations ();

    FortranOpenMPModuleDeclarations (std::string const & userSubroutineName,
        FortranParallelLoop * parallelLoop, SgScopeStatement * moduleScope);

  public:

    SgVariableDeclaration *
    getGlobalOpDatDeclaration (unsigned int OP_DAT_ArgumentGroup);

    SgVariableDeclaration *
    getFirstExecutionBooleanDeclaration ();
};

#endif