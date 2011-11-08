#pragma once
#ifndef CPP_OPENCL_USER_SUBROUTINE_H
#define CPP_OPENCL_USER_SUBROUTINE_H

#include <CPPUserSubroutine.h>

class CPPOpenCLUserSubroutine: public CPPUserSubroutine
{
  public:

    CPPOpenCLUserSubroutine (SgScopeStatement * moduleScope,
        CPPParallelLoop * parallelLoop,
        CPPProgramDeclarationsAndDefinitions * declarations);
};

#endif
