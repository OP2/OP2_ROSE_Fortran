#ifndef KERNEL_SUBROUTINE_H
#define KERNEL_SUBROUTINE_H

#include <Subroutine.h>
#include <UserDeviceSubroutine.h>
#include <ParallelLoop.h>

namespace
{
  std::string const variableName_threadidx = "threadidx";
  std::string const variableName_x = "x";
  std::string const variableName_blockidx = "blockidx";
  std::string const variableName_blockdim = "blockdim";
  std::string const variableName_griddim = "griddim";
}

class KernelSubroutine: public Subroutine
{
  protected:

    /*
     * ======================================================
     * The OP_DAT formal parameters
     * ======================================================
     */

    std::map <unsigned int, SgVariableDeclaration *> formalParameter_OP_DATs;

  protected:

    KernelSubroutine (std::string const & subroutineName) :
      Subroutine (subroutineName + "_kernel")
    {
    }
};

#endif
