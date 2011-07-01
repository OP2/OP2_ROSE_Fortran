#ifndef CPP_KERNEL_SUBROUTINE_H
#define CPP_KERNEL_SUBROUTINE_H

#include <KernelSubroutine.h>
#include <CPPParallelLoop.h>

class CPPKernelSubroutine: public KernelSubroutine <SgFunctionDeclaration,
    CPPProgramDeclarationsAndDefinitions>
{

  protected:

    CPPKernelSubroutine::CPPKernelSubroutine (
        std::string const & subroutineName,
        std::string const & userSubroutineName, CPPParallelLoop * parallelLoop) :
      KernelSubroutine <SgFunctionDeclaration, CPPProgramDeclarationsAndDefinitions> (
          subroutineName, userSubroutineName, parallelLoop)
    {
    }
};

#endif
