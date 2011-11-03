#pragma once
#ifndef CPP_CUDA_USER_SUBROUTINE_H
#define CPP_CUDA_USER_SUBROUTINE_H

#include <CPPUserSubroutine.h>

class CPPCUDAUserSubroutine: public CPPUserSubroutine
{
  public:

    CPPCUDAUserSubroutine (SgScopeStatement * moduleScope,
        CPPParallelLoop * parallelLoop,
        CPPProgramDeclarationsAndDefinitions * declarations);
};

#endif
