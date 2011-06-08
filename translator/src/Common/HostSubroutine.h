#ifndef HOST_SUBROUTINE_H
#define HOST_SUBROUTINE_H

#include <Subroutine.h>
#include <ParallelLoop.h>

class HostSubroutine : public Subroutine
{
  protected:

    std::string userSubroutineName;

    std::string kernelSubroutineName;

    ParallelLoop * parallelLoop;

  protected:

    HostSubroutine (std::string const & subroutineName,
        std::string const & userSubroutineName,
        std::string const & kernelSubroutineName, ParallelLoop * parallelLoop);
};

#endif
