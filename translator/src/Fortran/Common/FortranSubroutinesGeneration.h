#ifndef FORTRAN_SUBROUTINES_GENERATION_H
#define FORTRAN_SUBROUTINES_GENERATION_H

#include <rose.h>
#include <SubroutinesGeneration.h>
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

class FortranSubroutinesGeneration: public SubroutinesGeneration <
    FortranProgramDeclarationsAndDefinitions, FortranParallelLoop,
    FortranHostSubroutine>
{
  protected:

    virtual FortranHostSubroutine
        *
        createSubroutines (FortranParallelLoop * parallelLoop,
            std::string const & userSubroutineName,
            SgScopeStatement * moduleScope) = 0;

    /*
     * ======================================================
     * Adds the relevant library 'use' statements to the
     * generated module
     * ======================================================
     */

    virtual void
    addLibraries (SgModuleStatement * moduleStatement) = 0;

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
    addContains (SgScopeStatement * moduleScope);

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

    virtual void
    visit (SgNode * node);

  public:

    FortranSubroutinesGeneration (
        FortranProgramDeclarationsAndDefinitions * declarations,
        std::string const & fileExtension) :
      SubroutinesGeneration <FortranProgramDeclarationsAndDefinitions,
          FortranParallelLoop, FortranHostSubroutine> (declarations,
          fileExtension)
    {
    }
};

#endif
