#ifndef CPP_KERNEL_SUBROUTINE_H
#define CPP_KERNEL_SUBROUTINE_H

#include <KernelSubroutine.h>
#include <CPPParallelLoop.h>

class CPPKernelSubroutine: public KernelSubroutine <SgFunctionDeclaration,
    CPPDeclarations>
{

  protected:

    CPPKernelSubroutine::CPPKernelSubroutine (
        std::string const & subroutineName,
        std::string const & userSubroutineName, CPPParallelLoop * parallelLoop) :
      KernelSubroutine <SgFunctionDeclaration, CPPDeclarations> (
          subroutineName, userSubroutineName, parallelLoop)
    {
    }
};

#endif
