/*
 * Written by Adam Betts and Carlo Bertolli
 *
 * This class models module-scope declarations in Fortran
 */

#ifndef FORTRAN_MODULE_DECLARATIONS_H
#define FORTRAN_MODULE_DECLARATIONS_H

#include <string>
#include <rose.h>
#include <FortranParallelLoop.h>
#include <SubroutineVariableDeclarations.h>

class FortranModuleDeclarations
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
    SubroutineVariableDeclarations * variableDeclarations;

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
    createInitialiseConstantsBooleanDeclaration ();

    void
    createFirstExecutionBooleanDeclaration ();

    FortranModuleDeclarations (std::string const & userSubroutineName,
        FortranParallelLoop * parallelLoop, SgScopeStatement * moduleScope);

  public:

    SgVariableDeclaration *
    getFirstExecutionBooleanDeclaration ();

    SgVariableDeclaration *
    getReductionArrayHostDeclaration (unsigned int OP_DAT_ArgumentGroup);

    SgVariableDeclaration *
    getReductionArrayDeviceDeclaration (unsigned int OP_DAT_ArgumentGroup);

    SgVariableDeclaration *
    getGlobalOpDatDeclaration (unsigned int OP_DAT_ArgumentGroup);

    SubroutineVariableDeclarations *
    getAllDeclarations ();
};

#endif
