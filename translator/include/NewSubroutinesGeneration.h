/*
 * Written by Adam Betts and Carlo Bertolli
 *
 * This class scans each provided source file and looks
 * for OP_PAR_LOOP calls. When such a call is found, the
 * following occurs:
 *
 * 1) A new file is created to which generated subroutines
 * are output
 *
 * 2) The subroutines implementing the OP_PAR_LOOP on the
 * device are generated
 *
 * 3) The provided source files are modified to replace
 * calls to the OP_PAR_LOOP with the newly generated host
 * subroutine
 */

#ifndef NEW_SUBROUTINES_GENERATION_H
#define NEW_SUBROUTINES_GENERATION_H

#include <rose.h>
#include <CommonNamespaces.h>
#include <Declarations.h>
#include <ParallelLoop.h>
#include <HostSubroutine.h>
#include <UserDeviceSubroutine.h>
#include <KernelSubroutine.h>

class NewSubroutinesGeneration: public AstSimpleProcessing
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
    Declarations * declarations;

    /*
     * ======================================================
     * A mapping from a user kernel name to our internal
     * representation of an OP_PAR_LOOP
     * ======================================================
     */
    std::map <std::string, ParallelLoop *> parallelLoops;

  private:

    /*
     * ======================================================
     * Patches the calls to OP_PAR_LOOPs with the new
     * subroutine call
     * ======================================================
     */
    void
    patchOP_PAR_LOOPCalls (ParallelLoop & parallelLoop,
        UserDeviceSubroutine & userDeviceSubroutine,
        HostSubroutine & hostSubroutine, SgScopeStatement * scope,
        SgFunctionCallExp * functionCallExp);

    /*
     * ======================================================
     * Creates subroutines targeting OpenMP
     * ======================================================
     */
    void
    createOpenMPSubroutines (ParallelLoop & parallelLoop,
        std::string const & userSubroutineName,
        SgModuleStatement * moduleStatement, SgNode * node,
        SgFunctionCallExp * functionCallExp);

    /*
     * ======================================================
     * Creates subroutines targeting CUDA
     * ======================================================
     */
    void
    createCUDASubroutines (ParallelLoop & parallelLoop,
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
            ParallelLoop & parallelLoop);

    /*
     * ======================================================
     * Creates the Fortran/C/C++ source file to be unparsed
     * that contains the generated subroutines and declarations
     * ======================================================
     */
    SgSourceFile &
    createSourceFile (ParallelLoop & parallelLoop);

  public:

    /*
     * ======================================================
     * Constructor requires the Fortran/C++/C source files
     * (contained in the SgProject) to detect OP_PAR_LOOPs and
     * the declaration of OP2 variables (contained in
     * Declarations)
     * ======================================================
     */
    NewSubroutinesGeneration (SgProject * project, Declarations * declarations)
    {
      this->project = project;
      this->declarations = declarations;
    }

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
