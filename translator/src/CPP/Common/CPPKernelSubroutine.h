#ifndef CPP_KERNEL_SUBROUTINE_H
#define CPP_KERNEL_SUBROUTINE_H

#include <CPPSubroutine.h>
#include <ParallelLoop.h>

class CPPKernelSubroutine: public CPPSubroutine
{
  protected:

    std::string userSubroutineName;

    ParallelLoop * parallelLoop;

  protected:

    CPPKernelSubroutine (std::string const & subroutineName,
        std::string const & userSubroutineName, ParallelLoop * parallelLoop);
};

#endif
