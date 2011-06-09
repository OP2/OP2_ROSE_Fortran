#include <UserSubroutine.h>

/*
 * ======================================================
 * Protected functions
 * ======================================================
 */

UserSubroutine::UserSubroutine (std::string const & subroutineName,
    ParallelLoop * parallelLoop) :
  Subroutine (subroutineName + "_device")
{
  this->hostSubroutineName = hostSubroutineName;

  this->parallelLoop = parallelLoop;
}

/*
 * ======================================================
 * Public functions
 * ======================================================
 */

std::string const &
UserSubroutine::getHostSubroutineName ()
{
  return hostSubroutineName;
}
