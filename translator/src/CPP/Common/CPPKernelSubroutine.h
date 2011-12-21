#pragma once
#ifndef CPP_KERNEL_SUBROUTINE_H
#define CPP_KERNEL_SUBROUTINE_H

#include <KernelSubroutine.h>

class CPPParallelLoop;
class CPPUserSubroutine;

class CPPKernelSubroutine: public KernelSubroutine <SgFunctionDeclaration>
{

  protected:

    CPPKernelSubroutine (SgScopeStatement * moduleScope,
        CPPUserSubroutine * calleeSubroutine, CPPParallelLoop * parallelLoop);
};

#endif
