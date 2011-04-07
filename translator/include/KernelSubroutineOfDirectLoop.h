/*
 * Written by Adam Betts and Carlo Bertolli
 *
 * This class models the kernel subroutine for a direct loop.
 * Its declarations and statements do the following:
 *
 */

#ifndef KERNEL_SUBROUTINE_OF_DIRECT_LOOP_H
#define KERNEL_SUBROUTINE_OF_DIRECT_LOOP_H

#include <KernelSubroutine.h>

class KernelSubroutineOfDirectLoop: public KernelSubroutine
{
  private:

    SgVariableDeclaration * variable_iterationSet;

    /*
     * ======================================================
     * The following variables are provided as
     * part of the CUDA library
     * ======================================================
     */

    SgVarRefExp * variable_Threadidx;

    SgVarRefExp * variable_X;

    SgVarRefExp * variable_Blockidx;

    SgVarRefExp * variable_Blockdim;

    SgVarRefExp * variable_GridDim;

  private:

    /*
     * ======================================================
     * Creates the statements inside the kernel subroutine
     * ======================================================
     */
    void
    createStatements (UserDeviceSubroutine & userDeviceSubroutine,
        ParallelLoop & parallelLoop);

    /*
     * ======================================================
     * Creates local variables inside the kernel subroutine
     * ======================================================
     */
    void
    createLocalVariables ();

  public:

    KernelSubroutineOfDirectLoop (std::string const & subroutineName,
        UserDeviceSubroutine & userDeviceSubroutine,
        ParallelLoop & parallelLoop, SgScopeStatement * moduleScope);
};

#endif
