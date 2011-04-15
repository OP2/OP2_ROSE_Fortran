/*
 * Written by Adam Betts and Carlo Bertolli
 *
 * This class models the kernel subroutine for an indirect loop.
 * Its declarations and statements do the following:
 *
 */

#ifndef KERNEL_SUBROUTINE_OF_INDIRECT_LOOP_H
#define KERNEL_SUBROUTINE_OF_INDIRECT_LOOP_H

#include <KernelSubroutine.h>
#include <DeviceDataSizesDeclaration.h>

class KernelSubroutineOfIndirectLoop: public KernelSubroutine
{
  private:

    /*
     * ======================================================
     * The first formal parameter in an indirect loop is a
     * variable containing the size information of the other
     * formal parameters
     * ======================================================
     */

    SgVariableDeclaration * formalParameter_argsSizes;

    /*
     * ======================================================
     * The OP_MAP formal parameters
     * ======================================================
     */

    std::map <unsigned int, SgVariableDeclaration *> formalParameter_OP_MAPs;

  private:

    void
    create_OP_DAT_FormalParameters (ParallelLoop & parallelLoop);

    void
    createArgsSizesFormalParameter (
        DeviceDataSizesDeclaration & deviceDataSizesDeclaration);

  public:

    KernelSubroutineOfIndirectLoop (std::string const & subroutineName,
        UserDeviceSubroutine & userDeviceSubroutine,
        DeviceDataSizesDeclaration & deviceDataSizesDeclaration,
        ParallelLoop & parallelLoop, SgScopeStatement * moduleScope);

};

#endif
