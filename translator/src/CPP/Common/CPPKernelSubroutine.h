#ifndef CPP_KERNEL_SUBROUTINE_H
#define CPP_KERNEL_SUBROUTINE_H

#include <KernelSubroutine.h>
#include <ParallelLoop.h>

class CPPKernelSubroutine: public KernelSubroutine <SgFunctionDeclaration>
{

  protected:

    CPPKernelSubroutine (std::string const & subroutineName,
        std::string const & userSubroutineName, ParallelLoop * parallelLoop);
};

#endif
