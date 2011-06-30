#ifndef FORTRAN_SUBROUTINES_GENERATION_H
#define FORTRAN_SUBROUTINES_GENERATION_H

#include <rose.h>
#include <CommonNamespaces.h>
#include <Declarations.h>
#include <FortranParallelLoop.h>
#include <FortranCUDAHostSubroutine.h>
#include <FortranCUDAUserSubroutine.h>
#include <FortranCUDAKernelSubroutine.h>

class FortranSubroutinesGeneration: public AstSimpleProcessing
{
  private:

    /*
     * ======================================================
     * The project which the source-to-source translator
     * operates upon
     * ======================================================
     */
    SgProject * project;

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
    FortranDeclarations * declarations;

    /*
     * ======================================================
     * A mapping from a user kernel name to our internal
     * representation of an OP_PAR_LOOP
     * ======================================================
     */
    std::map <std::string, FortranParallelLoop *> parallelLoops;

  private:

    /*
     * ======================================================
     * Patches the calls to OP_PAR_LOOPs with the new
     * subroutine call
     * ======================================================
     */
    void
    patchCallsToParallelLoops (FortranParallelLoop & parallelLoop,
        std::string const & userSubroutineName,
        FortranHostSubroutine & hostSubroutine, SgScopeStatement * scope,
        SgFunctionCallExp * functionCallExp);

    /*
     * ======================================================
     * Creates subroutines targeting OpenMP
     * ======================================================
     */
    void
    createOpenMPSubroutines (FortranParallelLoop * parallelLoop,
        std::string const & userSubroutineName,
        SgModuleStatement * moduleStatement, SgNode * node,
        SgFunctionCallExp * functionCallExp);

    /*
     * ======================================================
     * Creates subroutines targeting CUDA
     * ======================================================
     */
    void
    createCUDASubroutines (FortranParallelLoop * parallelLoop,
        std::string const & userSubroutineName,
        SgModuleStatement * moduleStatement, SgNode * node,
        SgFunctionCallExp * functionCallExp);

    /*
     * ======================================================
     * Adds the relevant library 'use' statements to the
     * generated OpenMP module
     * ======================================================
     */
    void
    addOpenMPLibraries (SgModuleStatement * moduleStatement);

    /*
     * ======================================================
     * Adds the relevant library 'use' statements to the
     * generated CUDA module
     * ======================================================
     */
    void
    addCUDALibraries (SgModuleStatement * moduleStatement);

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
    SgModuleStatement
    *
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

    /*
     * ======================================================
     * Generates the new subroutines
     * ======================================================
     */
    FortranSubroutinesGeneration (SgProject * project,
        FortranDeclarations * declarations);
};

#endif
