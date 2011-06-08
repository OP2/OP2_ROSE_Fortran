#ifndef CPP_HOST_SUBROUTINE_H
#define CPP_HOST_SUBROUTINE_H

#include <HostSubroutine.h>
#include <CPPSubroutine.h>
#include <ParallelLoop.h>

class CPPHostSubroutine: public CPPSubroutine, public HostSubroutine
{
  protected:

    CPPHostSubroutine (std::string const & subroutineName,
        std::string const & userSubroutineName,
        std::string const & kernelSubroutineName, ParallelLoop * parallelLoop);
};

#endif
