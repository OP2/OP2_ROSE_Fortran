#ifndef CPP_OPENCL_HOST_SUBROUTINE_H
#define CPP_OPENCL_HOST_SUBROUTINE_H

#include <CPPHostSubroutine.h>

class CPPOpenCLHostSubroutine: public CPPHostSubroutine
{
  protected:

    CPPOpenCLHostSubroutine (std::string const & subroutineName,
        std::string const & userSubroutineName,
        std::string const & kernelSubroutineName,
        SgScopeStatement * moduleScope, CPPParallelLoop * parallelLoop);
};

#endif
