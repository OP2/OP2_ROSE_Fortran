#pragma once
#ifndef FORTRAN_SUBROUTINES_GENERATION_H
#define FORTRAN_SUBROUTINES_GENERATION_H

#include <SubroutinesGeneration.h>

class SgProject;
class SgSourceFile;
class SgModuleStatement;
class FortranReductionSubroutines;
class FortranOpDatDimensionsDeclaration;
class FortranModuleDeclarations;
class FortranHostSubroutine;
class FortranParallelLoop;
class FortranProgramDeclarationsAndDefinitions;

class FortranSubroutinesGeneration: public SubroutinesGeneration <
    FortranProgramDeclarationsAndDefinitions, FortranHostSubroutine>
{
  protected:

    FortranReductionSubroutines * reductionSubroutines;

    std::map <std::string, FortranOpDatDimensionsDeclaration *>
        dimensionsDeclarations;

    std::map <std::string, FortranModuleDeclarations *> moduleDeclarations;

    std::vector <std::string> headersWithAddedUseStatements;

  private:

    void
    removeUseStatement (SgUseStatement * lastUseStatement,
        std::string const & userSubroutineName);

    SgVariableDeclaration *
    addUserSubroutineNameDeclaration (SgScopeStatement * scope,
        std::string const & userSubroutineName);

    SgScopeStatement *
    addModuleUseStatement (SgNode * parent, std::string const & moduleName);

    void
    patchCallsToParallelLoops (std::string const & moduleName);

    /*
     * ======================================================
     * Adds the Fortran 'contains' statement to the module
     * which must precede all subroutine declarations
     * ======================================================
     */
    void
    addContains ();

    /*
     * ======================================================
     * Creates the Fortran module
     * ======================================================
     */
    SgModuleStatement *
    createFortranModule (std::string const & moduleName);

  protected:

    /*
     * ======================================================
     * Creates the new subroutines. This function is pure virtual
     * so the target backend has to implement it
     * ======================================================
     */
    virtual void
    createSubroutines () = 0;

    /*
     * ======================================================
     * Creates subroutines needed for reductions. This function
     * is pure virtual so the target backend has to implement it
     * ======================================================
     */
    virtual void
    createReductionSubroutines () = 0;

    /*
     * ======================================================
     * Creates the module type and variable declarations. This
     * function is pure virtual so the target backend has to
     * implement it
     * ======================================================
     */
    virtual void
    createModuleDeclarations () = 0;

    /*
     * ======================================================
     * Adds the relevant library 'use' statements to the
     * generated module. This function is pure virtual as the
     * needed libraries are backend specific
     * ======================================================
     */
    virtual void
    addLibraries () = 0;

    /*
     * ======================================================
     * Do the generation
     * ======================================================
     */
    void
    generate ();

  public:

    FortranSubroutinesGeneration (SgProject * project,
        FortranProgramDeclarationsAndDefinitions * declarations,
        std::string const & newFileName);
};

#endif
