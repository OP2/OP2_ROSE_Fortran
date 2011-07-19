#ifndef CPP_OPENCL_KERNEL_SUBROUTINE_H
#define CPP_OPENCL_KERNEL_SUBROUTINE_H

#include <CPPKernelSubroutine.h>

class CPPOpenCLKernelSubroutine: public CPPKernelSubroutine
{
  protected:

    CPPOpenCLKernelSubroutine (std::string const & subroutineName,
        std::string const & userSubroutineName, SgScopeStatement * moduleScope,
        CPPParallelLoop * parallelLoop);
};

#endif
