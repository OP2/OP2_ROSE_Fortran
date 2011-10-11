#pragma once
#ifndef FORTRAN_SUBROUTINES_GENERATION_H
#define FORTRAN_SUBROUTINES_GENERATION_H

#include <SubroutinesGeneration.h>

class SgSourceFile;
class SgModuleStatement;
class FortranReductionSubroutines;
class FortranOpDatDimensionsDeclaration;
class FortranModuleDeclarations;
class FortranHostSubroutine;
class FortranParallelLoop;
class FortranProgramDeclarationsAndDefinitions;

namespace Libraries
{
  std::string const ISO_C_BINDING = "ISO_C_BINDING";
  std::string const OP2_C = "OP2_C";
  std::string const CUDAFOR = "CUDAFOR";
  std::string const OMP_LIB = "OMP_LIB";
  std::string const cudaConfigurationParams = "cudaConfigurationParams";
}

class FortranSubroutinesGeneration: public SubroutinesGeneration <
    FortranProgramDeclarationsAndDefinitions, FortranHostSubroutine>
{

  protected:

    FortranReductionSubroutines * reductionSubroutines;

    std::map <std::string, FortranOpDatDimensionsDeclaration *>
        dimensionsDeclarations;

    std::map <std::string, FortranModuleDeclarations *> moduleDeclarations;

  private:

    void
    fixUseStatement (SgUseStatement * lastUseStatement,
        std::string const & userSubroutineName);

    void
    patchCallsToParallelLoops ();

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
    createFortranModule (SgSourceFile & sourceFile);

    /*
     * ======================================================
     * Creates the Fortran source file to be unparsed
     * that contains the generated subroutines and declarations
     * ======================================================
     */
    SgSourceFile &
    createSourceFile ();

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

    FortranSubroutinesGeneration (
        FortranProgramDeclarationsAndDefinitions * declarations,
        std::string const & fileSuffix);
};

#endif
