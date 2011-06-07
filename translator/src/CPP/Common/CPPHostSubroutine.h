#ifndef CPP_HOST_SUBROUTINE_H
#define CPP_HOST_SUBROUTINE_H

#include <CPPSubroutine.h>
#include <ParallelLoop.h>

class CPPHostSubroutine: public CPPSubroutine
{
  protected:

    std::string userSubroutineName;

    std::string kernelSubroutineName;

    ParallelLoop * parallelLoop;

  protected:

    CPPHostSubroutine (std::string const & subroutineName,
        std::string const & userSubroutineName,
        std::string const & kernelSubroutineName, ParallelLoop * parallelLoop);
};

#endif
