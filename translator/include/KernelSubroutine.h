#ifndef KERNEL_SUBROUTINE_H
#define KERNEL_SUBROUTINE_H

#include <Subroutine.h>
#include <UserDeviceSubroutine.h>
#include <ParallelLoop.h>

class KernelSubroutine: public Subroutine
{
  protected:

    /*
     * ======================================================
     * The OP_DAT formal parameters
     * ======================================================
     */

    std::map <unsigned int, SgVariableDeclaration *> formalParameter_OP_DATs;

    /*
     * ======================================================
     * The following variables are provided as part of the
     * CUDA library
     * ======================================================
     */

    SgVarRefExp * variable_Threadidx;

    SgVarRefExp * variable_X;

    SgVarRefExp * variable_Blockidx;

    SgVarRefExp * variable_Blockdim;

    SgVarRefExp * variable_GridDim;

  protected:

    KernelSubroutine (std::string const & subroutineName) :
      Subroutine (subroutineName + "_kernel")
    {
    }
};

#endif
