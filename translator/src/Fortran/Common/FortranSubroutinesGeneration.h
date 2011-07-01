#ifndef FORTRAN_SUBROUTINES_GENERATION_H
#define FORTRAN_SUBROUTINES_GENERATION_H

#include <rose.h>
#include <FortranProgramDeclarationsAndDefinitions.h>
#include <FortranParallelLoop.h>
#include <FortranHostSubroutine.h>

namespace Libraries
{
  std::string const ISO_C_BINDING = "ISO_C_BINDING";
  std::string const OP2_C = "OP2_C";
  std::string const CUDAFOR = "CUDAFOR";
  std::string const OMP_LIB = "OMP_LIB";
  std::string const cudaConfigurationParams = "cudaConfigurationParams";
}

class FortranSubroutinesGeneration: public AstSimpleProcessing
{
  protected:

    /*
     * ======================================================
     * The generated output files for each OP_PAR_LOOP
     * ======================================================
     */
    std::vector <SgSourceFile *> generatedFiles;

    /*
     * ======================================================
     * Used to retrieve base types and dimensions of OP_DAT
     * variables (and all other OP2 declarations, if needed)
     * and subroutines defined in the source files to be
     * processed
     * ======================================================
     */
    FortranProgramDeclarationsAndDefinitions * declarations;

    /*
     * ======================================================
     * A mapping from a user subroutine name to our internal
     * representation of an OP_PAR_LOOP
     * ======================================================
     */
    std::map <std::string, FortranParallelLoop *> parallelLoops;

  protected:

    /*
     * ======================================================
     * Creates the subroutines
     * ======================================================
     */
    virtual FortranHostSubroutine *
    createSubroutines (FortranParallelLoop * parallelLoop,
        std::string const & userSubroutineName,
        SgModuleStatement * moduleStatement) = 0;

    /*
     * ======================================================
     * Adds the relevant library 'use' statements to the
     * generated module
     * ======================================================
     */
    virtual void
    addLibraries (SgModuleStatement * moduleStatement) = 0;

    /*
     * ======================================================
     * Patches the calls to OP_PAR_LOOPs with a call to the
     * new host subroutine
     * ======================================================
     */
    void
    patchCallsToParallelLoops (FortranParallelLoop & parallelLoop,
        std::string const & userSubroutineName,
        FortranHostSubroutine & hostSubroutine, SgScopeStatement * scope,
        SgFunctionCallExp * functionCallExp);

    /*
     * ======================================================
     * Adds the Fortran 'contains' statement to the module
     * which must precede all subroutine declarations
     * ======================================================
     */
    void
    addContains (SgModuleStatement * moduleStatement);

    /*
     * ======================================================
     * Creates the Fortran module
     * ======================================================
     */
    SgModuleStatement *
    createFortranModule (SgSourceFile & sourceFile,
        FortranParallelLoop & parallelLoop);

    /*
     * ======================================================
     * Creates the Fortran/C/C++ source file to be unparsed
     * that contains the generated subroutines and declarations
     * ======================================================
     */
    SgSourceFile &
    createSourceFile (FortranParallelLoop & parallelLoop);

    FortranSubroutinesGeneration (
        FortranProgramDeclarationsAndDefinitions * declarations) :
      declarations (declarations)
    {
    }

  public:

    /*
     * ======================================================
     * Over-riding implementation of the AST vertex traversal
     * function
     * ======================================================
     */
    virtual void
    visit (SgNode * node);

    /*
     * ======================================================
     * Generates output files for each OP_PAR_LOOP
     * ======================================================
     */
    void
    unparse ();
};

#endif
