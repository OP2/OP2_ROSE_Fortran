#pragma once
#ifndef CPP_KERNEL_SUBROUTINE_H
#define CPP_KERNEL_SUBROUTINE_H

#include <KernelSubroutine.h>

class CPPParallelLoop;

class CPPKernelSubroutine: public KernelSubroutine <SgFunctionDeclaration>
{

  protected:

    CPPKernelSubroutine (std::string const & subroutineName,
        std::string const & userSubroutineName, SgScopeStatement * moduleScope,
        CPPParallelLoop * parallelLoop);
};

#endif
