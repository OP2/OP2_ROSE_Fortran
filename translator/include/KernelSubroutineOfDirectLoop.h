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

    /*
     * ======================================================
     * The first formal parameter in a direct loop is the size
     * of the OP_SET
     * ======================================================
     */

    SgVariableDeclaration * formalParameter_setSize;

    /*
     * ======================================================
     * A local variable to count
     * ======================================================
     */

    SgVariableDeclaration * variable_IterationCounter;

  private:

    /*
     * ======================================================
     * Creates the actual parameters for the call to the
     * device version of the user subroutine
     * ======================================================
     */
    SgStatement *
    createUserSubroutineCall (UserDeviceSubroutine & userDeviceSubroutine,
        ParallelLoop & parallelLoop);

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

    void
    create_OP_DAT_FormalParameters (ParallelLoop & parallelLoop);

    void
    createSetSizeFormalParameter ();

  public:

    KernelSubroutineOfDirectLoop (std::string const & subroutineName,
        UserDeviceSubroutine & userDeviceSubroutine,
        ParallelLoop & parallelLoop, SgScopeStatement * moduleScope);
};

#endif
