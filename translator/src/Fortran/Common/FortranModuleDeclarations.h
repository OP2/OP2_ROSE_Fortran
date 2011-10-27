#pragma once
#ifndef FORTRAN_MODULE_DECLARATIONS_H
#define FORTRAN_MODULE_DECLARATIONS_H

#include <string>

class ScopedVariableDeclarations;
class FortranParallelLoop;
class SgScopeStatement;
class SgVarRefExp;

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

    SgVarRefExp *
    getCPlanReturnDeclaration ();

    SgVarRefExp *
    getGlobalOpDatDeclaration (unsigned int OP_DAT_ArgumentGroup);
};

#endif
