#ifndef CPP_KERNEL_SUBROUTINE_H
#define CPP_KERNEL_SUBROUTINE_H

#include <KernelSubroutine.h>
#include <CPPSubroutine.h>
#include <ParallelLoop.h>

class CPPKernelSubroutine: public CPPSubroutine, public KernelSubroutine
{

  protected:

    CPPKernelSubroutine (std::string const & subroutineName,
        std::string const & userSubroutineName, ParallelLoop * parallelLoop);
};

#endif
