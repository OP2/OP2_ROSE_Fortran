/*
 * Written by Adam Betts and Carlo Bertolli
 *
 * This class models module-scope declarations in Fortran
 */

#pragma once
#ifndef FORTRAN_MODULE_DECLARATIONS_H
#define FORTRAN_MODULE_DECLARATIONS_H

#include <string>

class ScopedVariableDeclarations;
class FortranParallelLoop;
class SgScopeStatement;
class SgVariableDeclaration;

class FortranModuleDeclarations
{
  protected:

    /*
     * ======================================================
     * Variables declared at module scope level
     * ======================================================
     */
    ScopedVariableDeclarations * variableDeclarations;

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

  private:

    void
    createCPlanReturnDeclaration ();

  protected:

    FortranModuleDeclarations (FortranParallelLoop * parallelLoop,
        SgScopeStatement * moduleScope);

  public:

    SgVariableDeclaration *
    getCPlanReturnDeclaration ();

    SgVariableDeclaration *
    getReductionArrayHostDeclaration (unsigned int OP_DAT_ArgumentGroup);

    SgVariableDeclaration *
    getReductionArrayDeviceDeclaration (unsigned int OP_DAT_ArgumentGroup);

    SgVariableDeclaration *
    getGlobalOpDatDeclaration (unsigned int OP_DAT_ArgumentGroup);

    ScopedVariableDeclarations *
    getAllDeclarations ();
};

#endif
